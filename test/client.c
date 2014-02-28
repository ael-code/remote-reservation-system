#include <stdio.h>
#include <argp.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>



struct client_option{
	int server_port;  					// port of the server to contact
	char * server_ip;  					// ip of the server to contact
	unsigned short int colored;		// 0 = false; 1=true; dafault 0
};

     
error_t parse_opt (int key, char *arg, struct argp_state *state){
	int p;
	struct client_option * opt = state->input;
	switch (key){
		case 'c':
			opt ->colored = 1;
			break;
		case ARGP_KEY_ARG:
			switch (state->arg_num){
				case 0:
					opt->server_ip = arg;
					break;
				case 1:
					p=strtol(arg,NULL,10);
					if(p < 1 || p>65535){
						printf("ERROR: \"%s\" is not a valid port number\n",arg);
						exit(1);
					}		
					else
						opt->server_port = p;	
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
	struct client_option opt = {0};
	
	
	/*Parser section*/
	struct argp_option options[] = { 
		{"colored-output", 'c', 0, 0,"Colored output"},
		{ 0 }
	};
	struct argp argp = { options, parse_opt, "hostname port", 0 };
	argp_parse (&argp, argc, argv, 0, 0, &opt);
	/* End parser */
	
	printf("Server: %s\nPort:%d\n",opt.server_ip,opt.server_port);
}
