#include <stdio.h>
#include <argp.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

struct server_option{
	int port;  							// 0 means random port
	int backlog;  						// 50 default
	unsigned short int colored;	// 0 = false; 1=true; dafault 0
};

void * dispatcher_thread(void * arg){
	printf("daje\n");
	close((int *)arg);
}

void print_server_info(struct server_option *opt){
	printf("\n");
	if(opt->colored == 1){
		printf("\e[1;32mServer port:\e[0;34m %d\e[0m\n\e[1;32mBacklog:\e[0;34m %d\e[0m\n",opt->port,opt->backlog);
	}else{
		printf("Server port: %d\nBacklog: %d\n",opt->port,opt->backlog);
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
	switch (key){
		case 'p':
			opt->port = atoi(arg);
			break;
		case 'c':
			opt ->colored = 1;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

int main (int argc, char **argv){
	
	// 0 is a random port
	struct server_option sopt = {0,50,0};
	
	
	/*Parser section*/
	struct argp_option options[] = { 
		{"port", 'p', "PORT-NUM", 0, "Listening port"},
		{"colored-output", 'c', 0, 0,"Colored output"},
		{ 0 }
	};
	struct argp argp = { options, parse_opt, 0, 0 };
	argp_parse (&argp, argc, argv, 0, 0, &sopt);
	/* End parser */
	
	
	
	start_listen_thread(&sopt);
	
	
}


