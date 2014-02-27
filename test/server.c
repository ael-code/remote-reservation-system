#include <stdio.h>
#include <argp.h>

struct server_option{
int port;
};

static int parse_opt (int key, char *arg, struct argp_state *state){
	struct server_option * popt = state->input;
	switch (key){
		case 'p':
		{
			popt->port = atoi(arg);
			break;
		}
	}
	return 0;
}

int main (int argc, char **argv){
	struct server_option sopt = {-1};
	
	/*Parser section*/
	struct argp_option options[] = { 
		{"port", 'p', "port-number", 0, "Listening port"},
		{ 0 }
	};
	struct argp argp = { options, parse_opt, 0, 0 };
	argp_parse (&argp, argc, argv, 0, 0, &sopt);
	/* End parser */
	
	if(sopt.port == -1){
		printf("choosing port by myself... muahahah\n");
	}else{
		printf("try to start a server on port %d yourself!!\n", sopt.port);
	}
}
