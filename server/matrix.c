#include "matrix.h"
#include <stdlib.h>
#include <stdio.h>

 
char * mat;
unsigned int rows;
unsigned int cols; 

/*
*	Allocate memory and set dimension for mat
*/
void matrix_init(unsigned int mat_rows, unsigned int mat_cols){
	rows = mat_rows;
	cols = mat_cols;
	mat = (char * )calloc(sizeof(char),rows*cols);
	if(mat == NULL){printf("ERROR: calloc");exit(-1);}	
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
int seats_available2(unsigned int num, struct seat * seats){
	if(num < 1 || seats == NULL) return 0;
	
	struct seat * punt = seats;
	while( (punt-seats) < num ){
		//check bounds
		if(punt->row >= rows || punt -> col >= cols) return 0;
		//debug
		//printf("iteration %d: [%u,%u]=%d\n",punt-seats,punt->row,punt->col,(*(mat+(punt->row*cols)+punt->col))? 1:0);
		if( *(mat+(punt->row*cols)+punt->col) != 0)
			return 0;
		punt++;
	}
	return 1;
}
*/

/*
*	Set all cells in "seats".
*/
void occupy_seats(unsigned int num, struct seat * seats){
	//control if parameters are good
	if(num < 1 || seats == NULL){puts("bad parameter in occupy");exit(-1);}
	char (*matrix)[cols] =(char (*)[cols]) mat;
	struct seat * punt = seats;
	while( (punt-seats) < num ){
		//check bounds
		if(punt->row >= rows || punt -> col >= cols){puts("bad parameter in occupy");exit(-1);}
		matrix[punt->row][punt->col] = 1;
		punt++;
	}
}





