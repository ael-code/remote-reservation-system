//compile: gcc -o server.out server.c reservation.c chiavazione.c matrix.c ../lib/seats.c ../lib/conversion.c -pthread -O3
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "matrix.h"
#include "chiavazione.h"
#include "seats.h"
#include "rrs_protocol.h"
#include "reservation.h"
#include "server.h"
#include "file_op.h"
#include "threads.h"

//struct to store dispatcher thread paramenter
struct thread_param{
	int sok;
	struct sockaddr_in addr;
	char t_name[22]; //ip:port
	//15 for ip , 1 for ':', 5 for port, 1 for '\0';
};


//Global variable
struct server_option sopt;
static int ssok;
char load_from_file;


void close_routine(int s){
	#ifdef DEBUG
	puts("starting close_routine()");
	#endif
	//Saving on file before performing request, in case of crash: saving the server_opt struct isn't needed because it doesn't change
	extern struct res_entry * array;
	int res = save_reservation_array(sopt.map_rows*sopt.map_cols,array,get_chiavazione_length((sopt.map_rows*sopt.map_cols)-1,sopt.pwd_length));
	
	if(res == -1){puts("error saving on file");}
	if(sopt.verbose == 1){
		if(sopt.colored)printf("\e[1;91mSaved reservations on file\e[0m\n");
		else printf("Saved reservations on file\n");
	}
	
	kill_all_threads();
	if(sopt.verbose == 1){
		if(sopt.colored)printf("\e[1;91mClosed all threads\e[0m\n");
		else printf("Closed all threads\n");
	}
	matrix_close();
	reservation_close();
	if(sopt.verbose == 1){
		if(sopt.colored)printf("\e[1;91mRemoved all semaphores\e[0m\n");
		else printf("Removed all semaphores\n");
	}
	close(ssok);
	if(sopt.verbose == 1){
		if(sopt.colored)printf("\e[1;91mClosed main socket\e[0m\n");
		else printf("Closed main socket\n");
	}
	
	file_close();
	pthread_exit(NULL);
}

//thread cleanup
void clean_thread_param(void * thread_parameter){
	struct thread_param * t_param = (struct thread_param *) thread_parameter;
	close(t_param->sok);
	if(sopt.verbose == 1){
		(sopt.colored)?printf("\e[1;91m<-\e[0m "):printf("<- ");
		printf("Closed    %s\n",t_param->t_name);
	}
	free(thread_parameter);
}

/*
*	Thread to menage incoming connetcions
*/
void * dispatcher_thread(void * thread_parameter){
	int res;
	struct thread_param * t_param = (struct thread_param *) thread_parameter;
	
	//create name for thread
	inet_ntop(AF_INET,&t_param->addr.sin_addr,t_param->t_name,sizeof(t_param->addr));
	sprintf((t_param->t_name)+strlen(t_param->t_name),":%d",ntohs(t_param->addr.sin_port));
	pthread_setname_np(pthread_self(), t_param->t_name);
	
	//routine to cleanup thread_parameter called on pthread_exit
	pthread_cleanup_push(clean_thread_param,thread_parameter);
	
	// print info
	if(sopt.verbose == 1){
		(sopt.colored)?printf("\e[1;92m->\e[0m "):printf("-> ");
		printf("Connected %s\n",t_param->t_name);
	}
	
	//receive header
	char req_header[HEADER_DIM];
	res = recv(t_param->sok,req_header,HEADER_DIM,0);
	if(res == -1){perror("recive request header");del_thread(pthread_self());pthread_exit(NULL);}
	req_header[HEADER_DIM-1] = '\0';
	
	
	if(strcmp(req_header,"MAP_REQUEST") == 0){ //saving isn't needed because this request doesn't modify matrix
		//reply MAP_RESPONSE
		char resp[HEADER_DIM] = "MAP_RESPONSE";
		res = send(t_param->sok,resp,HEADER_DIM,0);
		if(res == -1){perror("send MAP_RESPONSE");del_thread(pthread_self());pthread_exit(NULL);}
		//send map dimension
		unsigned int dim[2];
		dim[0] = sopt.map_rows;
		dim[1] = sopt.map_cols;
		res = send(t_param->sok,dim,sizeof(dim),0);
		if(res == -1){perror("send map dimension");del_thread(pthread_self());pthread_exit(NULL);}
		//send map
		char * matrix = get_matrix();
		res = send(t_param->sok,matrix,sopt.map_rows*sopt.map_cols,0);
		if(res == -1){perror("send map");del_thread(pthread_self());pthread_exit(NULL);}			
	
	}else if(strcmp(req_header,"RESERVATION") == 0){
		//reply PREN_RESPONSE
		char resp[HEADER_DIM] = "RESV_RESPONSE";
		res = send(t_param->sok,resp,HEADER_DIM,0);
		if(res == -1){perror("send RESV_RESPONSE");del_thread(pthread_self());pthread_exit(NULL);}
		
		//receive number of seats
		unsigned int seats_num = 0;
		res = recv(t_param->sok,&seats_num,sizeof(seats_num),0);
		if(res < sizeof(seats_num)){
			if(res == -1)perror("receive number of seats");
			else puts("Error: recived invalid seats num");
			del_thread(pthread_self());pthread_exit(NULL);
		}
		
		//receive seats
		struct seat seats[seats_num];
		res = recv(t_param->sok,seats,sizeof(seats),0);
		if(res < sizeof(seats)){
			if(res == -1)perror("receive seats");
			else puts("Error: mismatch of seats number recived");
			del_thread(pthread_self());pthread_exit(NULL);
		}
		//Saving on file before performing request, in case of crash: saving the server_opt struct isn't needed because it doesn't change
		extern struct res_entry * array;
		save_reservation_array(sopt.map_rows*sopt.map_cols,array,get_chiavazione_length((sopt.map_rows*sopt.map_cols)-1,sopt.pwd_length));
		//Performing reservation request
		char * chiavazione = reservation_perform(seats_num,seats);
		
		if(chiavazione != NULL){
			//send confirmation
			char aff[HEADER_DIM] = "RESV_AFFERMATIVE";
			res = send(t_param->sok,aff,HEADER_DIM,0);
			if(res == -1){perror("send RESV_AFFERMATIVE");reservation_delete(chiavazione);del_thread(pthread_self());pthread_exit(NULL);}
			//send chiavazione dim
			unsigned int key_dim = strlen(chiavazione)+1;
			res = send(t_param->sok,&key_dim,sizeof(key_dim),0);
			if(res == -1){perror("send chiavazione dimension");reservation_delete(chiavazione);del_thread(pthread_self());pthread_exit(NULL);}
			//send chiavazione
			res = send(t_param->sok,chiavazione,key_dim,0);
			if(res < key_dim){
				if(res == -1)perror("send chiavazione");
				else puts("Error: sending chiavazione failed");
				reservation_delete(chiavazione);
				del_thread(pthread_self());pthread_exit(NULL);
			}
		}else{
			char neg[HEADER_DIM] = "RESV_NEGATIVE";
			res = send(t_param->sok,neg,HEADER_DIM,0);
			if(res == -1){perror("send RESV_NEGATIVE");del_thread(pthread_self());pthread_exit(NULL);}
		}
		
	}else if(strcmp(req_header,"CANCEL") == 0){
		//reply with response
		char resp[HEADER_DIM] = "CANC_RESPONSE";
		res = send(t_param->sok,resp,HEADER_DIM,0);
		if(res == -1){perror("send CANC_RESPONSE");del_thread(pthread_self());pthread_exit(NULL);}
		//receive chiavazione
		unsigned int chiav_dim = get_chiavazione_length(sopt.map_rows*sopt.map_cols-1,sopt.pwd_length);
		char chiavazione[chiav_dim+1];
		res = recv(t_param->sok,chiavazione,sizeof(chiavazione),0);
		if(res == -1){perror("receive chiavazione");del_thread(pthread_self());pthread_exit(NULL);}
		
		//Saving on file before performing request, in case of crash: saving the server_opt struct isn't needed because it doesn't change
		extern struct res_entry * array;
		save_reservation_array(sopt.map_rows*sopt.map_cols,array,get_chiavazione_length((sopt.map_rows*sopt.map_cols)-1,sopt.pwd_length));
		
		//Performing reservation delete request
		if(res < sizeof(chiavazione) || reservation_delete(chiavazione)){
			char confirm[HEADER_DIM] = "CANC_NEGATIVE";
			res = send(t_param->sok,confirm,HEADER_DIM,0);
			if(res == -1){perror("send CANCEL NEGATIVE");del_thread(pthread_self());pthread_exit(NULL);}
		}else{
			char confirm[HEADER_DIM] = "CANC_AFFERMATIVE";
			res = send(t_param->sok,confirm,HEADER_DIM,0);
			if(res == -1){perror("send CANCEL POSITIVE");del_thread(pthread_self());pthread_exit(NULL);}
		}
		
	}else{
		if(sopt.verbose == 1){
			printf("BAD REQUEST: %-20s\n",req_header);
		}
		char resp[HEADER_DIM] ="BAD_REQUEST";
		res = send(t_param->sok,resp,HEADER_DIM,0);
		if(res == -1){perror("send MAP_RESPONSE");del_thread(pthread_self());pthread_exit(NULL);}
	}

	pthread_cleanup_pop(1);
	del_thread(pthread_self());
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
	int res;
	struct sockaddr_in addr;
	pthread_t tid;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(sopt.port);	
	addr.sin_addr.s_addr = INADDR_ANY;
	
	ssok = socket(AF_INET,SOCK_STREAM,0);
	if(ssok == -1){perror("socket");close_routine(-1);}
	
	res = bind(ssok,(struct sockaddr *)&addr,sizeof(addr));
	if(res == -1){perror("bind");close_routine(-1);}
	
	//if i choose to use a random port, i need to retrive the random port
	if(sopt.port == 0){
		unsigned int addr_size = sizeof(addr);
		//update addr
		getsockname(ssok,(struct sockaddr *)&addr,&addr_size);
	}

	sopt.port = ntohs(addr.sin_port);
	
	print_server_info();
	
	res = listen(ssok,sopt.backlog);
	if(res == -1){perror("listen");close_routine(-1);}
	
	unsigned int size = sizeof(struct sockaddr_in);
	while(1){
		struct thread_param * t_param = calloc(1,sizeof(struct thread_param));
		if(t_param == NULL){perror("start_listen_thread: calloc");close_routine(-1);}
		
		t_param->sok = accept(ssok,(struct sockaddr *)&(t_param->addr),&size);
		if(t_param->sok == -1){perror("accept");close_routine(-1);}
		pthread_create(&tid,NULL,dispatcher_thread,t_param);
		add_thread(tid);
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
		case 'f':
			sopt.file = arg;
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
			}
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

int main (int argc, char **argv){
	
	//signal
	sigset_t set;
	if(sigfillset(&set)){ perror("filling set of signals"); exit(-1);}
	struct sigaction sig_act;
	sig_act.sa_handler = close_routine;
	sig_act.sa_mask = set;
	
	if(sigaction(SIGINT,&sig_act,NULL)){ perror("sigaction sigint"); exit(-1);}
	if(sigaction(SIGTERM,&sig_act,NULL)){ perror("sigaction sigint"); exit(-1);}
	if(sigaction(SIGABRT,&sig_act,NULL)){ perror("sigaction sigint"); exit(-1);}
	if(sigaction(SIGHUP,&sig_act,NULL)){ perror("sigaction sigint"); exit(-1);}
	if(sigaction(SIGQUIT,&sig_act,NULL)){ perror("sigaction sigint"); exit(-1);}
	if(sigaction(SIGILL,&sig_act,NULL)){ perror("sigaction sigint"); exit(-1);}
	
	//add main to the thread list
	add_thread(pthread_self());
	
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
		{"file", 'f', "FILE-NAME", 0,"Backup file"},
		{"verbose",'v',0,0,"Verbose output"},
		{"pwd-length", 's', "LENGTH", 0,"length of password used to generate reservation keys [default 8]"},
		{ 0 }
	};
	struct argp argp = { options, parse_opt, "rows cols", 0 };
	argp_parse (&argp, argc, argv, 0, 0, NULL);
	/* End parser */
	
	load_from_file = file_exist(sopt.file);
	
	//load config from file if exist
	if(sopt.file != NULL && load_from_file){
		load_server_opt();
		//print
		if(sopt.verbose){
			printf("server options succesfully loaded from file \"%s\"\n\n",sopt.file);
		}
	}else if(sopt.file != NULL && !load_from_file){
		save_server_opt();
		//print
		if(sopt.verbose){
			printf("server options succesfully saved on file \"%s\"\n\n",sopt.file);
		}
	}
	
	//seats initialization
	matrix_init(sopt.map_rows,sopt.map_cols);
	
	//memmory structure initialization
	reservation_init(sopt.map_rows*sopt.map_cols,sopt.pwd_length);
	
	if(sopt.file != NULL && load_from_file){
		puts("loading reservation");
		extern struct res_entry * array;
		if(load_reservation_array(sopt.map_rows*sopt.map_cols,array,get_chiavazione_length((sopt.map_rows*sopt.map_cols)-1,sopt.pwd_length))){
			puts("error loading reservation array from file");
			close_routine(-1);
		}
	}
	
	
	//thing pointed to by matrix is an array of map_cols char
	char (*matrix)[sopt.map_cols] =(char (*)[sopt.map_cols]) get_matrix();
	
	// debug seats
	if(sopt.colored != 0)
		print_SeatsMap_Colored(sopt.map_rows,sopt.map_cols,matrix);
	else
		print_SeatsMap_Special(sopt.map_rows,sopt.map_cols,matrix);
	
	start_listen_thread();
	
	close_routine(-1);
	return 0;
}
