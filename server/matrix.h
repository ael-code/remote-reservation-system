#ifndef MATRIX_H
#define MATRIX_H

#include "reservation.h"

/*
*	Allocate memory and set dimension for mat
*/
void matrix_init(unsigned int rows, unsigned int cols);

char * get_matrix();
/*
*	return 1 if cells in "seats" are all free. 0 instead.
*/
int seats_available(unsigned int num, struct seat * seats);
/*
*	Set all cells in "seats".
*/
void occupy_seats(unsigned int num, struct seat * seats);

#endif
