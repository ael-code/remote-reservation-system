#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>
#include "matrix.h"

 
char * mat;
unsigned int rows;
unsigned int cols;
int semid_m; //se metto nome uguale a quello di reservation.c da problemi
int res;

void printSem(char * msg);
/*
*	Allocate memory and set dimension for mat
*/
void matrix_init(unsigned int mat_rows, unsigned int mat_cols){
	
	//set dimensions
	rows = mat_rows;
	cols = mat_cols;
	
	//allocate memory for mat
	mat = (char * )calloc(sizeof(char),rows*cols);
	if(mat == NULL){printf("ERROR: calloc");exit(-1);}
	
	/*	IPC_PRIVATE isn't a flag field but a key_t type.  If this
   *   special value is used for key, the  system  call  ignores
   *   everything but the least significant 9 bits of semflg and
   *   creates a new semaphore set (on success).
	*/
	semid_m = semget(IPC_PRIVATE,rows*cols,IPC_CREAT|IPC_EXCL|0600);
	if(semid_m == -1){perror("semget in matrix_init()");exit(-1);}
	
	unsigned short vals[rows*cols];
	int i;
	for(i = 0;i<rows*cols;i++)
		vals[i]=1;
	
	res = semctl(semid_m, rows*cols, SETALL, vals);
	if(res == -1){perror("semctl in matrix_init()");exit(-1);}
}

char * get_matrix(){
	return mat;
}

/*
*	return 1 if all cells indicated by "seats" are free. (no input controls)
*/
int seats_available(unsigned int num, struct seat * seats){
	char (*matrix)[cols] =(char (*)[cols]) mat;
	struct seat * punt = seats;
	while( (punt-seats) < num ){
		if( matrix[punt->row][punt->col] != 0)
			return 0;
		punt++;
	}
	return 1;
}

/*  without the cast
		if( *(mat+(punt->row*cols)+punt->col) != 0)
*/

/*
*	Set to 1 all cells indicated by "seats". (no input controls)
*/
void occupy_seats(unsigned int num, struct seat * seats){
	char (*matrix)[cols] =(char (*)[cols]) mat;
	struct seat * punt = seats;
	while( (punt-seats) < num ){
		//paranoic control
		if(matrix[punt->row][punt->col] == 1){puts("founded seats already occupied");exit(-1);}
		matrix[punt->row][punt->col] = 1;
		punt++;
	}
}

/*
*	Set to 0 all cells indicated by "seats". (no input controls)
*/
void free_seats(unsigned int num, struct seat * seats){
	char (*matrix)[cols] =(char (*)[cols]) mat;
	struct seat * punt = seats;
	while( (punt-seats) < num ){
		if(matrix[punt->row][punt->col] == 0){puts("founded seats already free");exit(-1);}
		matrix[punt->row][punt->col] = 0;
		punt++;
	}
}

/*
*	Lock semaphores on these "seats". (no input controls)
*/
void lock_seats(unsigned int num, struct seat * seats){
	
	struct seat * punt = seats;
	struct sembuf sops[num];
	int i;
	for(i = 0; i<num; i++){
		sops[i].sem_num = ( punt[i].row * cols ) + punt[i].col ;
		sops[i].sem_op = -1;
		sops[i].sem_flg = 0;	
	}
	int res = semop(semid_m,sops,sizeof(sops)/sizeof(struct sembuf));
	if(res == -1){perror("semop, locking matrix semaphores");exit(-1);}
}

/*
*	Release semaphores on these "seats". (no input controls)
*/
void release_seats(unsigned int num, struct seat * seats){
	struct seat * punt = seats;
	struct sembuf sops[num];
	int i;
	for(i = 0; i<num; i++){
		sops[i].sem_num = ( punt[i].row * cols ) + punt[i].col ;
		sops[i].sem_op = 1;
		sops[i].sem_flg = 0;	
	}
	int res = semop(semid_m,sops,sizeof(sops)/sizeof(struct sembuf));
	if(res == -1){perror("semop, releasing matrix sempaphores");exit(-1);}
}

/*
*	If these "seats" violate constraints return (int > 0).
* 	# 1 if parameter aren't good
*	# 2 if the seats are out og bounds
* 	# 3 if seats array contains duplicate
*/
int control_seats(unsigned int num, struct seat * seats){
	//check parameter
	if(num < 1 || seats == NULL)
		return 1;
	
	struct seat * punt;
	struct seat * puntB;
	
	//check bounds
	punt = seats;
	while( (punt-seats) < num ){
		if(punt->row >= rows || punt -> col >= cols)
			return 2;
		punt++;
	}
	
	//check duplicate
	punt = seats;
	while((punt-seats)< num-1){
		puntB = punt+1;
		while((puntB-seats)<num){
			if(punt->row == puntB->row && punt->col == puntB->col)
				return 3;
			puntB++;
		}
		punt++;
	}
	
	return 0;
}

void printSem(char * msg){/* DEBUG */
	printf("%s\n",msg);	
	
	struct semid_ds sds;
	res = semctl(semid_m, 0,IPC_STAT,&sds);
	if(res == -1){perror("semctl in matrix_init()");exit(-1);}	
	printf("sem dim %ld\n",sds.sem_nsems);
	
	int i;
	for(i=0;i<rows*cols;i++)printf("sem[%d]= %d\n",i,semctl(semid_m, i, GETVAL));
}



