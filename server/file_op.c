#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "server.h"

static int res;
extern struct server_option sopt;

void save_server_opt(){

	int des_f = open(sopt.file,O_WRONLY|O_TRUNC|0660);
	if(des_f == -1){
		des_f = open(sopt.file,O_CREAT|O_WRONLY|O_TRUNC|0660);
		if(des_f == -1){perror("creating file");exit(-1);}
	}
		
	res = write(des_f,&sopt, sizeof(sopt));
	if(res < sizeof(sopt)){
		if(res == -1)
			perror("saving server option on file");
		else
			puts("error: saving server optiong on file");	
		exit(-1);
	}
	
	if(close(des_f) == -1)
		perror("closing file_ds in save_server_opt()");
}

void load_server_opt(){
	//save file name
	char * temp_name = sopt.file;
	
	int des_f = open(sopt.file,0660);
	if(des_f == -1){perror("loading server option from file");exit(-1);}
	
	res = read(des_f,&sopt, sizeof(sopt));
	if(res < sizeof(sopt)){
		if(res == -1)
			perror("loading server option from file");
		else
			puts("error: loading server option from file");	
		exit(-1);
	}
	
	//restore file name
	sopt.file = temp_name;
	
	if(close(des_f) == -1)
		perror("closing file_ds in load_server_opt()");
}

int file_exist(char * file){
	struct stat s;
	return stat(file,&s)? 0: 1;
}
