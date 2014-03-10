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

#define LINE_DIM 100

struct client_option{
	int server_port;  					// port of the server to contact
	char * server_ip;  					// ip of the server to contact
	char colored;							// 0 = false; 1=true; dafault 0
	char reserve;							// 0 = false; 1=true; dafault 0
	char verbose;							// 0 = false; 1=true; dafault 0
	char delete;							// 0 = false; 1=true; dafault 0
	char * chiavazione;
};

struct client_option opt;

void request_map(int sok){
	char req_header[HEADER_DIM] = "MAP_REQUEST";
	int res;
	
	//send request
	res = send(sok,req_header,HEADER_DIM,0);
	if(res == -1){perror("send");exit(-1);}
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mSent header:\e[0m %s\n",req_header);
		else printf("Sent header: %s\n",req_header);
	}
	
	//receive response header
	char res_header[HEADER_DIM];
	res = recv(sok,res_header,HEADER_DIM,0);
	if(res == -1){perror("recv header");exit(-1);}
	res_header[HEADER_DIM-1] = '\0';
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mReceived header:\e[0m %s\n",res_header);
		else printf("Received header: %s\n",res_header);
	}
	
	if(strcmp(res_header,"MAP_RESPONSE")!= 0){
		if(opt.colored) printf("\e[1;91mBAD RESPONSE:\e[0m %-*s\n\n",HEADER_DIM,res_header);
		else printf("BAD RESPONSE: %-*s\n\n",HEADER_DIM,res_header);
		exit(-1);
	}
		
	//receive map dimension
	unsigned int dim[2];
	res = recv(sok,dim,sizeof(dim),0);
	if(res < sizeof(dim)){
		if(res == -1)perror("recv dimension");
		else puts("Error: received invalid map dimension");
		exit(-1);
	}
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mReceived map dimension:\e[0m [%u,%u]\n",dim[0],dim[1]);
		else printf("Received map dimension: [%u,%u]\n",dim[0],dim[1]);
	}
	
	//receive map
	char map[dim[0]][dim[1]];
	res = recv(sok,map,dim[0]*dim[1],0);
	if(res < dim[0]*dim[1]){
		if(res == -1)perror("recv map");
		else puts("Received invalid map");
		exit(-1);
	}
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mReceived map data:\e[0m %d bytes\n",res);
		else printf("Received map data: %d bytes\n",res);
	}
	
	close(sok);
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mClosed socket.\e[0m\n");
		else printf("Closed socket.\n");
		printf("\n");
	}
	
	if(opt.colored != 0)
		print_SeatsMap_Colored(dim[0],dim[1],map);
	else
		print_SeatsMap_Special(dim[0],dim[1],map);
}

void reservation(int sok){
	char req_header[HEADER_DIM] = "RESERVATION";
	int res;
	
	//send request
	res = send(sok,req_header,HEADER_DIM,0);
	if(res == -1){perror("send");exit(-1);}
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mSent header:\e[0m %s\n",req_header);
		else printf("Sent header: %s\n",req_header);
	}
	
	//receive response header
	char res_header[HEADER_DIM];
	res = recv(sok,res_header,HEADER_DIM,0);
	if(res == -1){perror("recv header");exit(-1);}
	res_header[HEADER_DIM-1] = '\0';
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mReceived header:\e[0m %s\n",res_header);
		else printf("Received header: %s\n",res_header);
		printf("\n");
	}
	
	if(strcmp(res_header,"RESV_RESPONSE")!= 0){
		if(opt.colored) printf("\e[1;91mBAD RESPONSE:\e[0m %-*s\n\n",HEADER_DIM,res_header);
		else printf("BAD RESPONSE: %-*s\n\n",HEADER_DIM,res_header);
		exit(-1);
	}
	
	/* Seats input */
	
	char line[LINE_DIM];
	unsigned int seats_num;
	do{
		printf("Insert the number of seats you want to reserve: ");
		fflush(stdout);
		fgets(line,LINE_DIM,stdin);
		res = sscanf(line,"%u\n",&seats_num);
	}while(res < 1);
	
	if(seats_num == 0)exit(0);
	
	struct seat seats[seats_num];
	int i=0;
	while(i < seats_num){
		do{
			printf("Insert row and cols for seats[%d]: ",i);
			fflush(stdout);
			fgets(line,LINE_DIM,stdin);
			res = sscanf(line,"%u %u",&seats[i].row,&seats[i].col);
		}while(res<2);
		i++;
	}
	
	/* End Seats input */
	
	
	//send seats num
	res = send(sok,&seats_num,sizeof(seats_num),0);
	if(res == -1){perror("send");exit(-1);}
	
	//print
	if(opt.verbose){
		printf("\n");
		if(opt.colored) printf("\e[0;93mSent number of seats.\e[0m\n");
		else printf("Sent number of seats.\n");
	}
	
	//send seats
	res = send(sok,seats,sizeof(seats),0);
	if(res == -1){perror("send");exit(-1);}

	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mSent seats data:\e[0m %d bytes\n",res);
		else printf("Sent seats data: %d bytes\n",res);
	}	
	
	//receive confirmation
	char confirm[HEADER_DIM];
	res = recv(sok,confirm,HEADER_DIM,0);
	if(res == -1){perror("recv confirm");exit(-1);}
	res_header[HEADER_DIM-1] = '\0';
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mReceived header:\e[0m %s\n",confirm);
		else printf("Received header: %s\n",confirm);
	}
	
	//exit if response is not AFFERMATIVE
	if(strcmp(confirm,"RESV_AFFERMATIVE")!= 0){
		printf("\n");
		if(strcmp(confirm,"RESV_NEGATIVE") == 0){
			if(opt.colored)puts(" [\e[1;91m NO \e[0m] Reservation denied from server\n");
			else puts(" [ NO ] Reservation denied from server\n");
		}else{
			if(opt.colored) printf("\e[1;91mBAD RESPONSE:\e[0m %-*s\n\n",HEADER_DIM,confirm);
			else printf("BAD RESPONSE: %-*s\n\n",HEADER_DIM,confirm);
		}
		exit(-1);
	}
	
	
	//recive chiavazione_dim  (include '\0')
	unsigned int key_dim;
	res = recv(sok,&key_dim,sizeof(key_dim),0);
	if(res < sizeof(key_dim)){
		if(res == -1)perror("recv key dimension");
		else puts("Received invalid key dimension");
		exit(-1);
	}
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mReceived key dimension:\e[0m %u\n",key_dim);
		else printf("Received key dimension: %u\n",key_dim);
	}
	
	//recive chiavazione
	char chiavazione[key_dim];
	res = recv(sok,chiavazione,sizeof(chiavazione),0);
	if(res < sizeof(chiavazione)){
		if(res == -1)perror("recv key");
		else puts("Received invalid chiavazione");
		exit(-1);
	}
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mReceived key:\e[0m %d bytes\n",res);
		else printf("Received key: %d bytes\n",res);
	}
	
	close(sok);
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mClosed socket.\e[0m\n");
		else printf("Closed socket.\n");
	}
	
	printf("\n");
	if(opt.colored) printf(" [\e[1;32m OK \e[0m] Reservation complete: \e[1;32m%s\e[0m\n\n",chiavazione);
	else printf(" [ OK ] Reservation complete: %s\n\n",chiavazione);
}

void delete_reservation(int sok){
	char req_header[HEADER_DIM] = "CANCEL";
	int res;
	
	//send request
	res = send(sok,req_header,HEADER_DIM,0);
	if(res == -1){perror("send \"CANCEL\" header");exit(-1);}
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mSent header:\e[0m %s\n",req_header);
		else printf("Sent header: %s\n",req_header);
	}
	
	//receive response header
	char res_header[HEADER_DIM];
	res = recv(sok,res_header,HEADER_DIM,0);
	if(res == -1){perror("recv header");exit(-1);}
	res_header[HEADER_DIM-1] = '\0';
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mReceived header:\e[0m %s\n",res_header);
		else printf("Received header: %s\n",res_header);
	}
	
	if(strcmp(res_header,"CANC_RESPONSE")!= 0){
		if(opt.colored) printf("\e[1;91mBAD RESPONSE:\e[0m %-*s\n\n",HEADER_DIM,res_header);
		else printf("BAD RESPONSE: %-*s\n\n",HEADER_DIM,res_header);
		exit(-1);
	}
	
	//send request
	res = send(sok,opt.chiavazione,strlen(opt.chiavazione)+1,0);
	if(res == -1){perror("send key");exit(-1);}
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mSent key:\e[0m %u bytes\n",res);
		else printf("Sent key: %u bytes\n",res);
	}
	
	//receive confirmation
	char confirm[HEADER_DIM];
	res = recv(sok,confirm,HEADER_DIM,0);
	if(res == -1){perror("recv confirm");exit(-1);}
	res_header[HEADER_DIM-1] = '\0';
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mReceived header:\e[0m %s\n",confirm);
		else printf("Received header: %s\n",confirm);
	}
	
	close(sok);
	
	//print
	if(opt.verbose){
		if(opt.colored) printf("\e[0;93mClosed socket.\e[0m\n");
		else printf("Closed socket.\n");
		printf("\n");
	}
	
	//exit if response is not AFFERMATIVE
	if(strcmp(confirm,"CANC_AFFERMATIVE")!= 0){
		if(strcmp(confirm,"CANC_NEGATIVE") == 0){
			if(opt.colored)puts(" [\e[1;91m NO \e[0m] Cancellation denied from server\n");
			else puts(" [ NO ] Cancellation denied from server\n");
		}else{
		 	if(opt.colored)printf("\e[1;91mBAD RESPONSE:\e[0m %-*s\n\n",HEADER_DIM,confirm);
		 	else printf("BAD RESPONSE: %-*s\n\n",HEADER_DIM,confirm);
		}
		exit(-1);
	}
	
	if(opt.colored) printf(" [\e[1;32m OK \e[0m] Cancellation complete\n\n");
	else printf(" [ OK ] Cancellation complete\n\n");
}

int connect_to_server(){
	//print
	if(opt.verbose){
		if(opt.colored)
			printf("\e[0;93mConnecting:\e[0m %s:%d\n",opt.server_ip,opt.server_port);
		else
			printf("Connecting: %s:%d\n",opt.server_ip,opt.server_port);	
	}
	
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
		case 'v':
			opt.verbose = 1;
			break;
		case 'r':
			if(opt.delete) argp_failure(state,1,0,"--delete and --reserve are not compatible. Please choose only one.");
			opt.reserve = 1;
			break;
		case 'd':
			if(opt.reserve) argp_failure(state,1,0," --reserve and --delete are not compatible. Please choose only one.");
			opt.delete = 1;
			opt.chiavazione = arg;
			break;
		case ARGP_KEY_ARG:
			switch (state->arg_num){
				case 0:
					opt.server_ip = arg;
					break;
				case 1:
					p=strtol(arg,NULL,10);
					if(p < 1 || p>65535)argp_failure(state,1,0,"ERROR \"%s\" is not a valid port number\n",arg);		
					opt.server_port = p;	
					break;
			}
			break;
		case ARGP_KEY_END:
			printf ("\n");
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
		{"reserve", 'r', 0, 0,"Reserve some seats"},
		{"verbose",'v',0,0,"Verbose output"},
		{"delete", 'd', "CODE" , 0,"Request server to remove reservation whith this CODE"},
		{ 0 }
	};
	struct argp argp = { options, parse_opt, "hostname port", 0 };
	argp_parse (&argp, argc, argv, 0, 0, NULL);
	/* End parser */
	
	int sok = connect_to_server();
	
	if(opt.reserve)
		reservation(sok);
	else if(opt.delete)
		delete_reservation(sok);
	else
		request_map(sok);
		
	exit(0);	
}
