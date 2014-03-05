//compile: gcc client.c ../lib/seats.c ../lib/conversion.c -o client.out
#include <stdio.h>
#include <argp.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/rrs_protocol.h"
#include "../lib/seats.h"



struct client_option{
	int server_port;  					// port of the server to contact
	char * server_ip;  					// ip of the server to contact
	char colored;							// 0 = false; 1=true; dafault 0
};

struct client_option opt;

void request_map(int sok){
	char req_header[HEADER_DIM] = "MAP_REQUEST";
	int res;
	
	//send request
	res = send(sok,req_header,HEADER_DIM,0);
	if(res == -1){perror("send");exit(-1);}
	
	char res_header[HEADER_DIM];
	//receive response header
	res = recv(sok,res_header,HEADER_DIM,0);
	if(res == -1){perror("recv header");exit(-1);};
	if(strcmp(res_header,"MAP_RESPONSE")!= 0){
		printf("BAD RESPONSE: %s",res_header);
		exit(-1);
	}
	
	//receive map dimension
	unsigned int dim[2];
	res = recv(sok,dim,sizeof(dim),0);
	if(res == -1){perror("recv dimension");exit(-1);};
	
	//receive map
	char map[dim[0]][dim[1]];
	res = recv(sok,map,dim[0]*dim[1],0);
	if(res == -1){perror("recv map");exit(-1);};
	
	if(opt.colored != 0)
		printSeatsColored(dim[0],dim[1],map);
	else
		printSeatsSpecial(dim[0],dim[1],map);
}

int connect_to_server(){
	int sok,res;
	struct sockaddr_in addr;
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(opt.server_port);
	inet_aton(opt.server_ip,&addr.sin_addr);
	
	sok = socket(AF_INET,SOCK_STREAM,0);
	if(sok == -1){perror("socket");exit(-1);}
	
	res = connect(sok,(struct sockaddr *)&addr,sizeof(addr));
	if(res == -1){perror("connect");exit(-1);}
	
	return sok;
}

error_t parse_opt (int key, char *arg, struct argp_state *state){
	int p;
	switch (key){
		case 'c':
			opt.colored = 1;
			break;
		case ARGP_KEY_ARG:
			switch (state->arg_num){
				case 0:
					opt.server_ip = arg;
					break;
				case 1:
					p=strtol(arg,NULL,10);
					if(p < 1 || p>65535){
						printf("ERROR: \"%s\" is not a valid port number\n",arg);
						exit(1);
					}		
					else
						opt.server_port = p;	
					break;
			}
			break;
		case ARGP_KEY_END:
			if(state->arg_num < 2){
				printf("ERROR: too few arguments\n");
				argp_usage(state);
				exit(1);
			}
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}


int main (int argc, char **argv){
	
	// 0 is a random port
	opt.server_port = 0;
	opt.colored = 0;
	
	/*Parser section*/
	struct argp_option options[] = { 
		{"colored-output", 'c', 0, 0,"Colored output"},
		{ 0 }
	};
	struct argp argp = { options, parse_opt, "hostname port", 0 };
	argp_parse (&argp, argc, argv, 0, 0, NULL);
	/* End parser */
	
	printf("Server: %s\nPort:%d\n",opt.server_ip,opt.server_port);
	
	int sok = connect_to_server();
	request_map(sok);
		
	exit(0);	
}
