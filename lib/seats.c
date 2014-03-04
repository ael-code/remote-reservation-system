#include <stdio.h>
#include <stdlib.h>
#include "seats.h"
#include "conversion.h"

/*
* Print Seats like matrix of integers
*/
int printSeats(int rows, int cols,char mat[rows][cols]){
	int i,j;
	for(i=0; i<rows; i++){
		for(j=0; j<cols; j++){
			printf("%c ",(mat[i][j]==1)?'1':'0');
		}
		printf("\n");
	}
}

/*
* Print Seats like matrix of cels |__|
*/
int printSeatsSpecial(int rows, int cols,char mat[rows][cols]){
	int i,j;
	int char_num = int_to_charc(rows-1);
	
	printf("%*s",char_num+1,"");
	for(i=0; i<cols; i++){
		printf("  %d ", i);
	}
	printf("\n");
	
	printf("%*s",char_num+1,"");
	for(i=0; i<cols; i++){
		printf(" ___");
	}
	printf("\n");
	
	for(i=0; i<rows; i++){
	printf("%*d |",char_num, i);
		for(j=0; j<cols; j++){
			if(mat[i][j]==1)
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
int printSeatsColored(int rows, int cols,char mat[rows][cols]){
	int i,j;
	int char_num = int_to_charc(rows-1);
	
	//offset laterale
	printf("%*s",char_num+1,"");
	for(i=0; i<cols; i++){
		printf("  %d ", i);
	}
	printf("\n");
	
	printf("%*s",char_num+1,"");
	for(i=0; i<cols; i++){
		printf(" ___");
	}
	printf("\n");
	
	for(i=0; i<rows; i++){
	printf("%*d |",char_num, i);
		for(j=0; j<cols; j++){
			if(mat[i][j]==1)
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
int resetSeats(int rows, int cols, char mat[rows][cols]){
	int i,j;
	for(i=0; i<rows; i++)
		for(j=0; j<cols; j++){
			mat[i][j]=0;
		}
}
