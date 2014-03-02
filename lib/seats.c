#include <stdio.h>
#include <stdlib.h>
#include "seats.h"

/*
* Print Seats like matrix of integers
*/
int printSeats(int rows, int cols, int mat[rows][cols]){
	int i,j;
	for(i=0; i<rows; i++){
		for(j=0; j<cols; j++){
			printf("%d ",mat[i][j]);
		}
		printf("\n");
	}
}

/*
* Print Seats like matrix of cels |__|
*/
int printSeatsSpecial(int rows, int cols, int mat[rows][cols]){
	int i,j;
	printf("   ");
	for(i=0; i<cols; i++){
	printf("  %d ", i);
	}
	printf("\n");
	
	printf("   ");
	for(i=0; i<cols; i++){
	printf(" ___");
	}
	printf("\n");
	
	for(i=0; i<rows; i++){
	printf("%c  |", 65+i);
		for(j=0; j<cols; j++){
			if(mat[i][j])
				printf("_#_|");
			else
				printf("___|");
		}
		printf("\n");
	}
}

/*
* Print Seats like matrix of colored cels
*/
int printSeatsColored(int rows, int cols, int mat[rows][cols]){
	int i,j;
	//offset laterale
	printf("   ");
	for(i=0; i<cols; i++){
	printf("  %d ", i);
	}
	printf("\n");
	
	printf("   ");
	for(i=0; i<cols; i++){
	printf(" ___");
	}
	printf("\n");
	
	for(i=0; i<rows; i++){
	printf("%c  |", 65+i);
		for(j=0; j<cols; j++){
			if(mat[i][j])
				printf("\e[41m___\e[0m|");
			else
				printf("\e[42m___\e[0m|");
		}
		printf("\n");
	}
}

/*
* put all seats to 0;
*/
int resetSeats(int rows, int cols, int mat[rows][cols]){
	int i,j;
	for(i=0; i<rows; i++)
		for(j=0; j<cols; j++){
			mat[i][j] =0;
		}
}

/*
* Print information about seats map dimension
*/
int infoSeats (int rows, int cols, int mat[rows][cols]){
printf("rows: %d\ncols: %d\n",rows,cols);
}
