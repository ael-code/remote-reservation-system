#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "server.h"
#include "conversion.h"
#include "reservation.h"
#include "seats.h"
#include "matrix.h"

static int res;
extern struct server_option sopt;
static int des_f;


void save_server_opt(){

	des_f = open(sopt.file,O_RDWR|O_TRUNC|0660);
	if(des_f == -1){
		des_f = open(sopt.file,O_CREAT|O_RDWR|O_TRUNC|0660);
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
	
	//if(close(des_f) == -1)
	// perror("closing file_ds in save_server_opt()");
}

void load_server_opt(){
	//save file name
	char * temp_name = sopt.file;
	
	des_f = open(sopt.file,O_RDWR|0660);
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
	
	//if(close(des_f) == -1)
	//	perror("closing file_ds in load_server_opt()");
}

int file_exist(char * file){
	struct stat s;
	return stat(file,&s)? 0: 1;
}

void file_close(){
	printf("closing \"%s\"",sopt.file);//debug
	if(close(des_f) == -1)
		perror("closing file");
}

int save_reservation_array(unsigned int arr_dim, struct res_entry * arr,unsigned int chiav_dim){
	res = lseek(des_f,sizeof(struct server_option),SEEK_SET); 
	if(res == -1){ perror("lseek in save_reservation_array");return(-1);}
	printf("start saving from %d offset\n",res);//debug
	
	struct res_entry * punt = arr;
	while(punt - arr < arr_dim){
		
		//write s_num
		res = write(des_f,&(punt->s_num),sizeof(punt->s_num));
		if(res < sizeof(punt->s_num)){
			if(res == -1)
				perror("writing s_num on file");
			else
				puts("error: writing s_num on file");
			return(-1);
		}
		
		if(punt->s_num != 0){
				
			print_SeatsArray(punt->s_num,punt->seats); //debug
			//write chiavazione
			res = write(des_f,punt->chiavazione,chiav_dim);
			if(res < chiav_dim){
				if(res == -1)
					perror("writing chiavazione on file");
				else
					puts("error: writing chiavazione on file");
			return(-1);
			}
		
			//write seats arr
			res = write(des_f,punt->seats,(punt->s_num)*(sizeof(struct seat)));
			if(res < (punt->s_num)*(sizeof(struct seat))){
				if(res == -1)
					perror("writing seats on file");
				else
					puts("error: writing seats on file");
				return(-1);
			}
		}
		
		punt++;
	}
	return 0;
}

int load_reservation_array(unsigned int arr_dim, struct res_entry * arr,unsigned int chiav_dim){
	res = lseek(des_f,sizeof(struct server_option),SEEK_SET); 
	if(res == -1){ perror("lseek in save_reservation_array");return(-1);}
	printf("start loading from %d offset\n",res);//debug
	
	struct res_entry * punt = arr;
	while(punt - arr < arr_dim){
		
		//read s_num
		res = read(des_f,&(punt->s_num),sizeof(punt->s_num));
		if(res < sizeof(punt->s_num)){
			if(res == -1)
				perror("reading s_num from file");
			else
				puts("error: reading s_num from file");
		return(-1);
		}
		
		if(punt->s_num != 0){
			
			//read chiavazione
			punt->chiavazione = malloc(chiav_dim+1);
			if(punt->chiavazione == NULL){perror("error in malloc load_reservation_array");return(-1);}
						
			res = read(des_f,punt->chiavazione,chiav_dim);
			if(res < chiav_dim){
				if(res == -1)
					perror("reading chiavazione from file");
				else
					puts("error: reading chiavazione from file");
			return(-1);
			}
		
			//read seats arr
			punt->seats = malloc((punt->s_num)*(sizeof(struct seat)));
			if(punt->seats == NULL){perror("error in malloc load_reservation_array");return(-1);}
			
			res = read(des_f,punt->seats,(punt->s_num)*(sizeof(struct seat)));
			if(res < (punt->s_num)*(sizeof(struct seat))){
				if(res == -1)
					perror("reading seats from file");
				else
					puts("error: reading seats from file");
				return(-1);
			}
			
			print_SeatsArray(punt->s_num,punt->seats);//debug
			//refill matrix
			occupy_seats(punt->s_num,punt->seats);
		}
		punt++;	
	}
	update_freep();
	return 0;
}

