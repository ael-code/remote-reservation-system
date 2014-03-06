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
* Print Seats like matrix of integers
*/
int printSeats(int rows, int cols,char mat[rows][cols]);
/*
* Print Seats like matrix of cels |__|
*/
int printSeatsSpecial(int rows, int cols,char mat[rows][cols]);
/*
* Print Seats like matrix of colored cels
*/
int printSeatsColored(int rows, int cols,char mat[rows][cols]);
/*
* put all seats to 0;
*/
int resetSeats(int rows, int cols, char mat[rows][cols]);

#endif
