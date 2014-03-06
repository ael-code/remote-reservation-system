#ifndef SEATS_H
#define SEATS_H

/*
*	row and coloumn of a seat
*/
struct seat{
	unsigned int row;
	unsigned int col;
};

/*
* Print SeatsMap like matrix of integers
*/
void print_SeatsMap(int rows, int cols,char mat[rows][cols]);

/*
* Print SeatsMap like matrix of cels |__|
*/
void print_SeatsMap_Special(int rows, int cols,char mat[rows][cols]);

/*
* Print SeatsMap like matrix of colored cels
*/
void print_SeatsMap_Colored(int rows, int cols,char mat[rows][cols]);
/*
* Print SeatsArray
*/
void print_SeatsArray(unsigned int num,struct seat * seats);

#endif
