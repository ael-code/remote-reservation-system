#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "server.h"
#include "conversion.h"
#include "reservation.h"
#include "seats.h"
#include "matrix.h"

static int res;
extern struct server_option sopt;
static int des_f;
static int des_sem;

int file_exist(char * file){
	struct stat s;
	return stat(file,&s)? 0: 1;
}

int file_close(){
	int res = semctl(des_sem,0,IPC_RMID);
	if(res == -1){perror("deleting des_sem in file_op.c");return -1;}
	
	if(close(des_f) == -1){
		perror("file_open(): closing file");
		return -1;
	}
	return 0;
}

int file_open(){
	des_f = open(sopt.file,O_RDWR);
	if(des_f == -1){
		des_f = open(sopt.file,O_CREAT|O_RDWR,0660);
		if(des_f == -1){perror("file_open(): creating file");return(-1);}
	}
	des_sem = semget(IPC_PRIVATE,1,0600);
	if(des_sem == -1){
		perror("file_open(): installing semaphore");
		return -1;
	}
	int res = semctl(des_sem, 0, SETVAL, 1);
	if(res == -1){perror("semctl in file_open()");return(-1);}

	return 0;
}

void save_server_opt(){
		
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
}

int save_reservation_array(unsigned int arr_dim, struct res_entry * arr,unsigned int chiav_dim){
	
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
				
			//write chiavazione
			res = write(des_f,punt->chiavazione,chiav_dim+1);
			if(res < chiav_dim+1){
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
	//res = lseek(des_f,sizeof(struct server_option),SEEK_SET); 
	//if(res == -1){ perror("lseek in load_reservation_array");return(-1);}
	
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
						
			res = read(des_f,punt->chiavazione,chiav_dim+1);
			if(res < chiav_dim+1){
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
			
			//refill matrix
			occupy_seats(punt->s_num,punt->seats);
		}
		punt++;	
	}
	update_freep(0);
	return 0;
}
static int file_enter(){
	struct sembuf sops;
	sops.sem_num = 0;
	sops.sem_op =	-1;
	sops.sem_flg = 0;
	int res = semop(des_sem,&sops,1);
	if(res == -1){perror("semop, file enter");return(-1);}
	
	return 0;
}
static int file_exit(){
	struct sembuf sops;
	sops.sem_num = 0;
	sops.sem_op =	1;
	sops.sem_flg = 0;
	int res = semop(des_sem,&sops,1);
	if(res == -1){perror("semop, file exit");return(-1);}

	return 0;
}
int save_delta_del(unsigned int index){
	if(file_enter() ==-1) return -1;
	
	//write operation character
	char op = 'D';
	res = write(des_f,&op,sizeof(op));
	if(res < sizeof(op)){
		if(res == -1)
			perror("writing op char on file");
		else
			puts("error: writing op char on file");
		return -1;
	}
	
	//write index
	res = write(des_f,&index,sizeof(index));
	if(res < sizeof(index)){
		if(res == -1)
			perror("writing index on file");
		else
			puts("error: writing index on file");
		return -1;
	}
	if(file_exit()==-1) return -1;
	return 0;	
}

int save_delta_add(unsigned int index, struct res_entry * reservation){
	if(file_enter() ==-1) return -1;
	
	//write operation character
	char op = 'A';
	res = write(des_f,&op,sizeof(op));
	if(res < sizeof(op)){
		if(res == -1)
			perror("writing op char on file");
		else
			puts("error: writing op char on file");
		return -1;
	}
	
	//write index
	res = write(des_f,&index,sizeof(index));
	if(res < sizeof(index)){
		if(res == -1)
			perror("writing index on file");
		else
			puts("error: writing index on file");
		return -1;
	}
	
	//write s_num
	res = write(des_f,&(reservation->s_num),sizeof(reservation->s_num));
	if(res < sizeof(reservation->s_num)){
		if(res == -1)
			perror("writing s_num on file");
		else
			puts("error: writing s_num on file");
		return -1;
	}
	
	//write seats
	res = write(des_f,reservation->seats,sizeof(struct seat)*reservation->s_num);
	if(res < sizeof(struct seat)*reservation->s_num){
		if(res == -1)
			perror("writing seats on file");
		else
			puts("error: writing seats on file");
		return(-1);
	}
	
	//write chiavazione
	res = write(des_f,reservation->chiavazione,sopt.chiavazione_length+1);
	if(res < sopt.chiavazione_length+1){
		if(res == -1)
			perror("writing chiavazione on file");
		else
			puts("error: writing chiavazione on file");
		return(-1);
	}
	
	if(file_exit()==-1) return -1;

	return 0;		
}

//this function expetcs file pointer positioned at the beginning of delta entries in file
int load_delta(){
	char op;
	unsigned int index;
	//read first char until file is terminated
	while((res = read(des_f,&op,sizeof(op))) != 0){
		if(res < sizeof(op)){
			if(res == -1)
				perror("reading delta op from file");
			else
				puts("error: reading delta op from file");
			return(-1);
		}
		
		switch (op){
			case 'A':;
				
				struct res_entry temp_entry;
				
				//read index from delta op
				res = read(des_f,&index,sizeof(index));
				if(res < sizeof(index)){
					if(res == -1)
						perror("reading index from file");
					else
						puts("error: reading index from file");
					return(-1);
				}
				
				//read seats_num from delta op
				res = read(des_f, &temp_entry.s_num, sizeof(temp_entry.s_num));
				if(res<sizeof(temp_entry.s_num)){
					if(res == -1)
						perror("reading seats_num from file");
					else
						puts("error: reading seats_num from file");
					return(-1);
				}				
								
				//read seats_arr from delta op
				int size_seats = temp_entry.s_num * sizeof(struct seat);
				temp_entry.seats = malloc(size_seats);
				res = read(des_f, temp_entry.seats, size_seats);
				if(res < size_seats){
					if(res == -1)
						perror("reading seats from file");
					else
						puts("error: reading seats from file");
					return(-1);
				}
				
				//read chiavazione from delta op
				temp_entry.chiavazione = malloc(sopt.chiavazione_length+1);
				if(temp_entry.chiavazione == NULL){perror("error in malloc load_delta");return(-1);}
				
				res = read(des_f, temp_entry.chiavazione, sopt.chiavazione_length+1);
				if(res < sopt.chiavazione_length+1){
					if(res == -1)
						perror("reading chiavazione from file");
					else
						puts("error: reading chiavazione from file");
					return(-1);
				}
				
				insert_res_in_array(index,&temp_entry);
				
			break;
			case 'D':;
				
				res = read(des_f,&index,sizeof(index));
				if(res < sizeof(index)){
					if(res == -1)
						perror("reading index from file");
					else
						puts("error: reading index from file");
					return(-1);
				}
				
				remove_res_from_array(index);		
			
			break;
			default:
				puts("invalid character read from file");
				return(-1);
		}
	}
	
	//refresh file
	//delete all but server sopt from file (also file pointer)
	res = ftruncate(des_f, sizeof(struct server_option));
	if(res == -1){
		perror("ftruncate error");
		return -1;
	}
	res = lseek(des_f,sizeof(struct server_option),SEEK_SET); 
	if(res == -1){ 
		perror("lseek in load_delta");
		return(-1);
	}
	extern struct res_entry * reserv_array;
	res = save_reservation_array(sopt.map_rows*sopt.map_cols,reserv_array,sopt.chiavazione_length);
	if(res){
		puts("Error file_op.c: save_reservation_array() in load_delta");
		return -1;
	}
	
	return 0;
}

