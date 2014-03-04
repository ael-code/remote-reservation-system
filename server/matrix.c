#include "matrix.h"
#include <stdlib.h>
#include <stdio.h>

char * mat;
unsigned int rows;
unsigned int cols; 

void matrix_init(unsigned int rows, unsigned int cols){
	mat = (char * )calloc(sizeof(char),rows*cols);
	if(mat == NULL){printf("ERROR: calloc");exit(-1);}	
}

char * get_matrix(){
	return mat;
}

