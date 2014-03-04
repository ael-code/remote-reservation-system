//compile: gcc -o server.out server.c reservation.c chiavazione.c matrix.c ../lib/seats.c ../lib/conversion.c -pthread -O3
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "matrix.h"
#include "../lib/rrs_protocol.h"
#include "../lib/seats.h"
#include "reservation.h"
#include <signal.h>

#define HEADER_DIM 20

//struct to store server option
struct server_option{
	int port;  								// 0 means random port
	int backlog;  							// 50 default
	char colored;							// 0 = false; 1=true; dafault 0
	char verbose;							// 0 = false; 1=true; default 0
	unsigned int pwd_length;
	unsigned int map_rows;
	unsigned int map_cols;
};

//struct to store dispatcher thread paramenter
struct thread_param{
	int sok;
	struct sockaddr_in addr;
};


//Global variable
struct server_option sopt;


//thread cleanup
void clean_thread_param(void * thread_parameter){
	if(sopt.verbose == 1){printf("# Closing thread\n");}
	free(thread_parameter);
}

/*
*	Thread to menage incoming connetcions
*/

void * dispatcher_thread(void * thread_parameter){
	
	struct thread_param * t_param = (struct thread_param *) thread_parameter;
	
	//routine to cleanup thread_parameter called on pthread_exit
	pthread_cleanup_push(clean_thread_param,thread_parameter);
	
	//store client ip
	char ip[16];
	strcpy(ip,inet_ntoa(t_param->addr.sin_addr));
	
	char req_header[HEADER_DIM];
	int res;
	
	// print info
	if(sopt.verbose == 1){
		printf("# Incoming connection\n   Ip: %s\n   Port: %d\n\n",ip,ntohs(t_param->addr.sin_port));
	}
	
	//recive header
	res = recv(t_param->sok,(void *)req_header,HEADER_DIM,0);
	
	if(res == -1){
		printf("Error: client sok\n");
		pthread_exit(NULL);
	}
	req_header[res] = '\0';
	
	if(strcmp(req_header,"MAP_REQUEST\n") == 0){
		//reply MAP_RESPONSE
		char * resp="MAP_RESPONSE";
		res = send(t_param->sok,resp,strlen(resp),0);
		if(res == -1){perror("send MAP_RESPONSE");pthread_exit(NULL);}
		//send map dimension
		unsigned int dim[2];
		dim[0] = sopt.map_rows;
		dim[1] = sopt.map_cols;
		res = send(t_param->sok,dim,sizeof(dim),0);
		if(res == -1){perror("send map dimension");pthread_exit(NULL);}
		//send map
		char * matrix = get_matrix();
		res = send(t_param->sok,matrix,sopt.map_rows*sopt.map_cols,0);
		if(res == -1){perror("send map dimension");pthread_exit(NULL);}			
	}else{
		char * resp ="BAD_REQUEST";
		res = send(t_param->sok,resp,strlen(resp),0);
		if(res == -1){perror("send MAP_RESPONSE");pthread_exit(NULL);}
	}
	
	pthread_cleanup_pop(1);
	pthread_exit(NULL);
}

void print_server_info(){
	printf("\n");
	if(sopt.colored == 1){
		printf("\e[1;32mServer port:\e[0;34m %d\e[0m\n\e[1;32mBacklog:\e[0;34m %d\e[0m\n\e[1;32mRows:\e[0;34m %d\e[0m\n\e[1;32mCols:\e[0;34m %d\e[0m\n\e[1;32mpwd_length:\e[0;34m %d\e[0m\n\n",sopt.port,sopt.backlog,sopt.map_rows,sopt.map_cols,sopt.pwd_length);
	}else{
		printf("Server port: %d\nBacklog: %d\nRows: %d\nCols: %d\npwd_length: %d\n\n",sopt.port,sopt.backlog,sopt.map_rows,sopt.map_cols,sopt.pwd_length);
	}
}

int start_listen_thread(){
	signal(SIGPIPE,SIG_IGN);
	int ssok,res;
	struct sockaddr_in addr;
	struct sockaddr_in inaddr;
	pthread_t tid;
	
	print_server_info();
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(sopt.port);	
	addr.sin_addr.s_addr = INADDR_ANY;
	
	ssok = socket(AF_INET,SOCK_STREAM,0);
	if(ssok == -1){perror("socket");return(-1);}
	
	res = bind(ssok,(struct sockaddr *)&addr,sizeof(addr));
	if(res == -1){perror("bind");return(-2);}
	
	res = listen(ssok,sopt.backlog);
	if(res == -1){perror("listen");return(-3);}
	
	int size = sizeof(struct sockaddr_in);
	while(1){
		struct thread_param * t_param = calloc(1,sizeof(struct thread_param));
		if(t_param == NULL){perror("start_listen_thread: calloc");exit(-5);}
		
		t_param->sok = accept(ssok,(struct sockaddr *)&(t_param->addr),&size);
		if(t_param->sok == -1){perror("accept");return(-4);}
		pthread_create(&tid,NULL,dispatcher_thread,t_param);
	}
}

error_t parse_opt (int key, char *arg, struct argp_state *state){
	int temp;
	switch (key){
		case 'p':
			temp = strtol(arg,NULL,10);
			if(temp < 1){
				printf("ERROR: \"%s\" is not a valid port number\n",arg);
				exit(1);
			}else{
				sopt.port = atoi(arg);
			}break;
		case 'c':
			sopt.colored = 1;
			break;
		case 'v':
			sopt.verbose = 1;
			break;
		case 's':
			temp = strtol(arg,NULL,10);
			if(temp < 1){
				printf("ERROR: \"%s\" is not a valid pwd length\n",arg);
				exit(1);
			}else{
				sopt.pwd_length = temp;
			}break;
		case ARGP_KEY_ARG:
			switch (state->arg_num){
				case 0:
					temp = strtol(arg,NULL,10);
					if(temp < 1){
						printf("ERROR: \"%s\" is not a valid rows number\n",arg);
						exit(1);
					}else{
						sopt.map_rows = temp;
					}break;
				case 1:
					temp = strtol(arg,NULL,10);
					if(temp < 1){
						printf("ERROR: \"%s\" is not a valid cols number\n",arg);
						exit(1);
					}else{
						sopt.map_cols = temp;
					}break;
			}break;
		case ARGP_KEY_END:
			if(state->arg_num < 2){
				printf("ERROR: too few arguments\n");
				argp_usage(state);
				exit(1);
			}
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

int main (int argc, char **argv){

	// server_option initialization (default)
	sopt.port = 0;
	sopt.backlog = 50;
	sopt.colored =0;
	sopt.verbose =0;
	sopt.pwd_length = 8;
	
	/*Parser section*/
	struct argp_option options[] = { 
		{"port", 'p', "PORT-NUM", 0, "Listening port"},
		{"colored-output", 'c', 0, 0,"Colored output"},
		{"verbose",'v',0,0,"Verbose output"},
		{"pwd-length", 's', "LENGTH", 0,"length of password used to generate reservation keys [default 8]"},
		{ 0 }
	};
	struct argp argp = { options, parse_opt, "rows cols", 0 };
	argp_parse (&argp, argc, argv, 0, 0, NULL);
	/* End parser */
	
	//seats initialization
	matrix_init(sopt.map_rows,sopt.map_cols);
	
	//memmory structure initialization
	reservation_init(sopt.map_rows*sopt.map_cols,sopt.pwd_length);
	
	//thing pointed to by matrix is an array of map_cols char
	char (*matrix)[sopt.map_cols] =(char (*)[sopt.map_cols]) get_matrix();
	
	//debug reservation 
	char * res_test = reservation_perform(5,NULL);
	printf("%s\n",res_test);
	printf("%d\n",reservation_delete(res_test));
	printf("%d\n",reservation_delete(res_test));
	
	
	//debug seats
	if(sopt.colored != 0)
		printSeatsColored(sopt.map_rows,sopt.map_cols,matrix);
	else
		printSeatsSpecial(sopt.map_rows,sopt.map_cols,matrix);
		
	start_listen_thread(&sopt);	
}
