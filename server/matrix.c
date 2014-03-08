#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>
#include "matrix.h"

 
char * mat;
unsigned int rows;
unsigned int cols;
int semid;
int res;


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
	semid = semget(IPC_PRIVATE,1,0600);
	if(semid == -1){perror("semget in matrix_init()");exit(-1);}
	res = semctl(semid, 0, SETVAL, 1);
	if(res == -1){perror("semctl in matrix_init()");exit(-1);}	
}

char * get_matrix(){
	return mat;
}

/*
*	return 1 if cells in "seats" are all free. 0 instead.
*/
int seats_available(unsigned int num, struct seat * seats){
	//control if parameters are good
	if(num < 1 || seats == NULL) return 0;
	char (*matrix)[cols] =(char (*)[cols]) mat;
	struct seat * punt = seats;
	while( (punt-seats) < num ){
		//check bounds
		if(punt->row >= rows || punt -> col >= cols) return 0;
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
*	Set all cells in "seats" at 1.
*/
void occupy_seats(unsigned int num, struct seat * seats){
	//control if parameters are good
	if(num < 1 || seats == NULL){puts("bad parameter in occupy_seats()");exit(-1);}
	char (*matrix)[cols] =(char (*)[cols]) mat;
	struct seat * punt = seats;
	while( (punt-seats) < num ){
		//check bounds
		if(punt->row >= rows || punt -> col >= cols){puts("bad parameter in occupy_seats()");exit(-1);}
		matrix[punt->row][punt->col] = 1;
		punt++;
	}
}

/*
*	Set all cells in "seats" at 0.
*/
void free_seats(unsigned int num, struct seat * seats){
	//control if parameters are good
	if(num < 1 || seats == NULL){puts("bad parameter in free_seats()");exit(-1);}
	char (*matrix)[cols] =(char (*)[cols]) mat;
	struct seat * punt = seats;
	while( (punt-seats) < num ){
		//check bounds
		if(punt->row >= rows || punt -> col >= cols){puts("bad parameter in free_seats()");exit(-1);}
		matrix[punt->row][punt->col] = 0;
		punt++;
	}
}





