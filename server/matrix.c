#include "matrix.h"
#include <stdlib.h>
#include <stdio.h>

 
char * mat;
unsigned int rows;
unsigned int cols; 

/*
*	Allocate memory and set dimension for mat
*/
void matrix_init(unsigned int rows, unsigned int cols){
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
	char (*mat)[cols] =(char (*)[cols]) mat;
	struct seat * punt;
	while( (punt-seats) < num ){
		if( mat[punt->row][punt->col] != 0)
			return 0;
		punt++;
	}
	return 1;
}

/*
*	Set all cells in "seats".
*/
void occupy_seats(unsigned int num, struct seat * seats){
	char (*mat)[cols] =(char (*)[cols]) mat;
	struct seat * punt;
	while( (punt-seats) < num ){
		mat[punt->row][punt->col] = 1;
		punt++;
	}
}





