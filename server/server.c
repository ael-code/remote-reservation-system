//compile: gcc -o server.out server.c reservation.c chiavazione.c ../lib/seats.c -pthread -lm
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../lib/rrs_protocol.h"
#include "../lib/seats.h"
#include "reservation.h"

//struct to store server option
struct server_option{
	int port;  								// 0 means random port
	int backlog;  							// 50 default
	unsigned short int colored;		// 0 = false; 1=true; dafault 0
	unsigned int pwd_length;
	unsigned int map_rows;
	unsigned int map_cols;
};

void * dispatcher_thread(void * arg){
	printf("daje\n");
	close((int *)arg);
}

void print_server_info(struct server_option *opt){
	printf("\n");
	if(opt->colored == 1){
		printf("\e[1;32mServer port:\e[0;34m %d\e[0m\n\e[1;32mBacklog:\e[0;34m %d\e[0m\n\e[1;32mRows:\e[0;34m %d\e[0m\n\e[1;32mCols:\e[0;34m %d\e[0m\n",opt->port,opt->backlog,opt->map_rows,opt->map_cols);
	}else{
		printf("Server port: %d\nBacklog: %d\nRows: %d\nCols: %d\n",opt->port,opt->backlog,opt->map_rows,opt->map_cols);
	}
}

int start_listen_thread(struct server_option *opt){
	int ssok,sok,res;
	struct sockaddr_in addr;
	struct sockaddr_in inaddr;
	pthread_t tid;
	
	print_server_info(opt);
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(opt->port);	
	addr.sin_addr.s_addr = INADDR_ANY;
	
	ssok = socket(AF_INET,SOCK_STREAM,0);
	if(sok == -1){perror("socket");return(-1);}
	
	res = bind(ssok,(struct sockaddr *)&addr,sizeof(addr));
	if(res == -1){perror("bind");return(-2);}
	
	res = listen(ssok,opt->backlog);
	if(res == -1){perror("listen");return(-3);}
	
	int size = sizeof(struct sockaddr_in);
	while(1){
		sok = accept(ssok,(struct sockaddr *)&inaddr,&size);
		if(sok == -1){perror("accept");return(-4);}
		pthread_create(&tid,NULL,dispatcher_thread,&sok);
	}
}

error_t parse_opt (int key, char *arg, struct argp_state *state){
	struct server_option * opt = state->input;
	int temp;
	switch (key){
		case 'p':
			opt->port = atoi(arg);
			break;
		case 'c':
			opt->colored = 1;
			break;
		case 's':
			temp = strtol(arg,NULL,10);
			if(temp < 1){
				printf("ERROR: \"%s\" is not a valid pwd length\n",arg);
				exit(1);
			}else{
				opt-> pwd_length = temp;
			}break;
		case ARGP_KEY_ARG:
			switch (state->arg_num){
				case 0:
					temp = strtol(arg,NULL,10);
					if(temp < 1){
						printf("ERROR: \"%s\" is not a valid rows number\n",arg);
						exit(1);
					}else{
						opt->map_rows = temp;
					}break;
				case 1:
					temp = strtol(arg,NULL,10);
					if(temp < 1){
						printf("ERROR: \"%s\" is not a valid cols number\n",arg);
						exit(1);
					}else{
						opt->map_cols = temp;
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
		
	// server_option initialization
	struct server_option sopt = {0,50,0,8};
	
	/*Parser section*/
	struct argp_option options[] = { 
		{"port", 'p', "PORT-NUM", 0, "Listening port"},
		{"colored-output", 'c', 0, 0,"Colored output"},
		{"pwd-length", 's', "LENGTH", 0,"length of password used to generate reservation keys [default 8]"},
		{ 0 }
	};
	struct argp argp = { options, parse_opt, "rows cols", 0 };
	argp_parse (&argp, argc, argv, 0, 0, &sopt);
	/* End parser */
	
	//seats initialization
	int seats[sopt.map_rows][sopt.map_cols];
	resetSeats(sopt.map_rows,sopt.map_cols,seats);
	
	//memmory structure initialization
	reservation_init(sopt.map_rows*sopt.map_cols,sopt.pwd_length);
	
	//debug reservation 
	printf("%s\n",reservation_perform(5,NULL));
	
	//debug seats
	if(sopt.colored != 0)
		printSeatsColored(sopt.map_rows,sopt.map_cols,seats);
	else
		printSeatsSpecial(sopt.map_rows,sopt.map_cols,seats);
		
	start_listen_thread(&sopt);	
}
