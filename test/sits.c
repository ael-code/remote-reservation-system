 #include <stdio.h> //printf
 #include <stdlib.h>



int printSeats(int rows, int cols, int mat[rows][cols]){
	int i,j;
	for(i=0; i<rows; i++){
		for(j=0; j<cols; j++){
			printf("%d ",mat[i][j]);
		}
		printf("\n");
	}
}

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

int resetSeats(int rows, int cols, int mat[rows][cols]){
	int i,j;
	for(i=0; i<rows; i++)
		for(j=0; j<cols; j++){
			mat[i][j] =0;
		}
}

int infoSeats (int rows, int cols, int mat[rows][cols]){
printf("rows: %d\ncols: %d\n",rows,cols);
}

int main (int argc, char ** argv){

	printf("started\n");
	int rows = 8;
	int cols = 10;
	int seats[rows][cols];
	
	resetSeats(rows,cols,seats);
	
	seats[3][4] = 1;
	seats[6][2] = 1;
	seats[5][1] = 1;
	seats[2][7] = 1;
	seats[5][6] = 1;
	seats[5][5] = 1;
	
	infoSeats(rows,cols,seats);
	printf("\n");
	printSeatsColored(rows,cols,seats);
	printf("\n");
	
	exit(0);
}
