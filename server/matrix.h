#ifndef __MATRIX__
#define __MATRIX__

#include "../lib/seats.h"

/*
*	Deallocate memory and remove semaphores
*/
void matrix_close();
/*
*	Allocate memory and set dimension for mat
*/
void matrix_init(unsigned int mat_rows, unsigned int mat_cols);

/*
*	return pointer to the matrix
*/
char * get_matrix();
/*
*	return 1 if all cells indicated by "seats" are free. (no input controls)
*/
int seats_available(unsigned int num, struct seat * seats);

/*  without the cast
		if( *(mat+(punt->row*cols)+punt->col) != 0)
*/

/*
*	Set to 1 all cells indicated by "seats". (no input controls)
*/
void occupy_seats(unsigned int num, struct seat * seats);

/*
*	Set to 0 all cells indicated by "seats". (no input controls)
*/
void free_seats(unsigned int num, struct seat * seats);

/*
*	Try to lock semaphores on these "seats". 
* 	This operation is blocking for T_OUT time, after which it returns -1.
*  Prevent Starvation
*/
int lock_seats(unsigned int num, struct seat * seats);

/*
*	Locks semaphores on these "seats" in a special way.
* 	Should be called only after a lock_seats() time-out.
*
* 		- Increase master semaphore (temporary deny access to other non stucked threads)
*		- Wait for my turn on time-out semaphore (temporary deny access to other stucked threads)
*		- Locks semaphores on these "seats".
*/
void exclusive_lock_seats(unsigned int num, struct seat * seats);

/*
*	Release semaphores on these "seats". (no input controls)
*/
void release_seats(unsigned int num, struct seat * seats);

/*
*	Release semaphores on these "seats" in a special way.
* 	Should be called only after a special_lock_seats().
*
*		- Release semaphores on these "seats".
*		- Restore to 1 time-out semaphore (allow access to others stucked threads)
* 		- Increase master semaphore (allow access to other non stucked threads)
*/
void exclusive_release_seats(unsigned int num, struct seat * seats);

/*
*	Wait for master semaphore becomes 0;
*/
void wait_master_semaphore();

/*
*	If these "seats" violate constraints return (int > 0).
* 	# 1 if parameter aren't good
*	# 2 if the seats are out og bounds
* 	# 3 if seats array contains duplicate
*/
int control_seats(unsigned int num, struct seat * seats);

#endif
