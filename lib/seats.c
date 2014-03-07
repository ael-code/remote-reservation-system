#include <stdio.h>
#include <stdlib.h>
#include "seats.h"
#include "conversion.h"

/*
* Print SeatsMap like matrix of integers
*/
void print_SeatsMap(int rows, int cols,char mat[rows][cols]){
	int i,j;
	for(i=0; i<rows; i++){
		for(j=0; j<cols; j++){
			printf("%c ",(mat[i][j]==1)?'1':'0');
		}
		printf("\n");
	}
	printf("\n");
}

/*
* Print SeatsMap like matrix of cels |__|
*/
void print_SeatsMap_Special(int rows, int cols,char mat[rows][cols]){
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
	printf("\n");
}

/*
* Print SeatsMap like matrix of colored cels
*/
void print_SeatsMap_Colored(int rows, int cols,char mat[rows][cols]){
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
	printf("\n");
}

/*
* Print SeatsArray
*/
void print_SeatsArray(unsigned int num,struct seat * seats){
	if(num < 1 || seats == NULL) return;
	
	struct seat * punt = seats;
	while((punt-seats) < num){
		printf("[%u,%u] ",punt->row,punt->col);
		punt++;
	}
	printf("\n");
}

