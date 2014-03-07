//compile: gcc -o server.out server.c reservation.c chiavazione.c matrix.c ../lib/seats.c ../lib/conversion.c -pthread -O3
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include "matrix.h"
#include "../lib/rrs_protocol.h"
#include "../lib/seats.h"
#include "reservation.h"


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
	char ip[16];
};


//Global variable
struct server_option sopt;


//thread cleanup
void clean_thread_param(void * thread_parameter){
	struct thread_param * t_param = (struct thread_param *) thread_parameter;
	close(t_param->sok);
	if(sopt.verbose == 1){
		(sopt.colored)?printf("\e[1;91m<-\e[0m "):printf("<- ");
		printf("Closed    %s:%d\n",t_param->ip,ntohs(t_param->addr.sin_port));
	}
	free(thread_parameter);
}

/*
*	Thread to menage incoming connetcions
*/
void * dispatcher_thread(void * thread_parameter){
	int res;
	struct thread_param * t_param = (struct thread_param *) thread_parameter;
	
	//store client ip
	inet_ntop(AF_INET,&t_param->addr.sin_addr,t_param->ip,sizeof(t_param->ip));
	
	//routine to cleanup thread_parameter called on pthread_exit
	pthread_cleanup_push(clean_thread_param,thread_parameter);
	
	// print info
	if(sopt.verbose == 1){
		(sopt.colored)?printf("\e[1;92m->\e[0m "):printf("-> ");
		printf("Connected %s:%d\n",t_param->ip,ntohs(t_param->addr.sin_port));
	}
	
	//recive header
	char req_header[HEADER_DIM];
	res = recv(t_param->sok,req_header,HEADER_DIM,0);
	if(res == -1){perror("recive request header");pthread_exit(NULL);}
	req_header[HEADER_DIM-1] = '\0';
	
	
	if(strcmp(req_header,"MAP_REQUEST") == 0){
		//reply MAP_RESPONSE
		char resp[HEADER_DIM] = "MAP_RESPONSE";
		res = send(t_param->sok,resp,HEADER_DIM,0);
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
	
	}else if(strcmp(req_header,"RESERVATION") == 0){
		//reply PREN_RESPONSE
		char resp[HEADER_DIM] = "RESV_RESPONSE";
		res = send(t_param->sok,resp,HEADER_DIM,0);
		if(res == -1){perror("send RESV_RESPONSE");pthread_exit(NULL);}
		
		//receive number of seats
		unsigned int seats_num = 0;
		res = recv(t_param->sok,&seats_num,sizeof(seats_num),0);
		if(res < sizeof(seats_num)){
			if(res == -1)perror("receive number of seats");
			else puts("Error: recived invalid seats num");
			pthread_exit(NULL);
		}
		
		//receive seats
		struct seat seats[seats_num];
		res = recv(t_param->sok,seats,sizeof(seats),0);
		if(res < sizeof(seats)){
			if(res == -1)perror("receive seats");
			else puts("Error: mismatch of seats number recived");
			pthread_exit(NULL);
		}
		
		//DEBUG:
		//printf("%u\n",seats_num);
		//print_SeatsArray(seats_num,seats);
		
		char * chiavazione = reservation_perform(seats_num,seats);
		
		if(chiavazione != NULL){
			//send confirmation
			char aff[HEADER_DIM] = "RESV_AFFERMATIVE";
			res = send(t_param->sok,aff,HEADER_DIM,0);
			if(res == -1){perror("send RESV_AFFERMATIVE");reservation_delete(chiavazione);pthread_exit(NULL);}
			//send chiavazione dim
			unsigned int key_dim = strlen(chiavazione)+1;
			res = send(t_param->sok,&key_dim,sizeof(key_dim),0);
			if(res == -1){perror("send chiavazione dimension");reservation_delete(chiavazione);pthread_exit(NULL);}
			//send chiavazione
			res = send(t_param->sok,chiavazione,key_dim,0);
			if(res < key_dim){
				if(res == -1)perror("send chiavazione");
				else puts("Error: sending chiavazione failed");
				reservation_delete(chiavazione);
				pthread_exit(NULL);
			}
		}else{
			char neg[HEADER_DIM] = "RESV_NEGATIVE";
			res = send(t_param->sok,neg,HEADER_DIM,0);
			if(res == -1){perror("send RESV_NEGATIVE");pthread_exit(NULL);}
		}
		
	}else if(strcmp(req_header,"CANCEL") == 0){
		//reply with response
		char resp[HEADER_DIM] = "CANC_RESPONSE";
		res = send(t_param->sok,resp,HEADER_DIM,0);
		if(res == -1){perror("send CANC_RESPONSE");pthread_exit(NULL);}
		//receive chiavazione
		unsigned int chiav_dim = get_chiavazione_length(sopt.map_rows*sopt.map_cols-1,sopt.pwd_length);
		char chiavazione[chiav_dim+1];
		res = recv(t_param->sok,chiavazione,sizeof(chiavazione),0);
		if(res == -1){perror("receive chiavazione");pthread_exit(NULL);}
		
		if(res < sizeof(chiavazione) || reservation_delete(chiavazione)){
			char confirm[HEADER_DIM] = "CANC_NEGATIVE";
			res = send(t_param->sok,confirm,HEADER_DIM,0);
			if(res == -1){perror("send CANCEL NEGATIVE");pthread_exit(NULL);}
		}else{
			char confirm[HEADER_DIM] = "CANC_AFFERMATIVE";
			res = send(t_param->sok,confirm,HEADER_DIM,0);
			if(res == -1){perror("send CANCEL POSITIVE");pthread_exit(NULL);}
		}
		
	}else{
		if(sopt.verbose == 1){
			printf("BAD REQUEST: %-20s\n",req_header);
		}
		char resp[HEADER_DIM] ="BAD_REQUEST";
		res = send(t_param->sok,resp,HEADER_DIM,0);
		if(res == -1){perror("send MAP_RESPONSE");pthread_exit(NULL);}
	}
	
	pthread_cleanup_pop(1);
	pthread_exit(NULL);
}

void print_server_info(){
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
	
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(sopt.port);	
	addr.sin_addr.s_addr = INADDR_ANY;
	
	ssok = socket(AF_INET,SOCK_STREAM,0);
	if(ssok == -1){perror("socket");return(-1);}
	
	res = bind(ssok,(struct sockaddr *)&addr,sizeof(addr));
	if(res == -1){perror("bind");return(-2);}
	
	//if i choose to use a random port, i need to retrive the random port
	if(sopt.port == 0){
		int addr_size = sizeof(addr);
		//update addr
		getsockname(ssok,(struct sockaddr *)&addr,&addr_size);
	}

	sopt.port = ntohs(addr.sin_port);
	
	print_server_info();
	
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
			if(temp < 1) argp_failure(state,1,0,"ERROR \"%s\" is not a valid port number\n",arg);
			sopt.port = atoi(arg);
			break;
		case 'c':
			sopt.colored = 1;
			break;
		case 'v':
			sopt.verbose = 1;
			break;
		case 's':
			temp = strtol(arg,NULL,10);
			if(temp < 1)argp_failure(state,1,0,"ERROR \"%s\" is not a valid pwd length\n",arg);
			sopt.pwd_length = temp;
			break;
		case ARGP_KEY_ARG:
			switch (state->arg_num){
				case 0:
					temp = strtol(arg,NULL,10);
					if(temp < 1)argp_failure(state,1,0,"ERROR \"%s\" is not a valid rows number\n",arg);
					sopt.map_rows = temp;
					break;
				case 1:
					temp = strtol(arg,NULL,10);
					if(temp < 1)argp_failure(state,1,0,"ERROR \"%s\" is not a valid cols number\n",arg);
					sopt.map_cols = temp;
					break;
			}break;
		case ARGP_KEY_END:
			printf ("\n");
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
	
	//debug seats
	if(sopt.colored != 0)
		print_SeatsMap_Colored(sopt.map_rows,sopt.map_cols,matrix);
	else
		print_SeatsMap_Special(sopt.map_rows,sopt.map_cols,matrix);
		
	start_listen_thread();	
}
