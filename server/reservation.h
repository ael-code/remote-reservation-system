#ifndef __RESERVATION__
#define __RESERVATION__

/*
*	this is the unit that rapresent one reservation in reservation array
*/
struct res_entry{
	int s_num; 					// number of seat involved in this reservation
	char * chiavazione; 		// pointer to chiavazione
	struct seat * seats;		// pointer to an array of seats (dimension s_num)
};

void update_freep(unsigned int index);

void reservation_close();

void reservation_init(unsigned int max_rese,unsigned int pwd_length);

char * reservation_perform(int s_num,struct seat * seats);

int reservation_delete(char * chiavazione);

struct res_entry * get_reservation(char * chiavazione);

/*
* insert this @reservation in array directly. It doesn't involve semaphores (use sequentially).
* Should be used ONLY to populate array during delta loading.
* Used by load_delta() in file_op.
*/
int insert_res_in_array(unsigned int index, struct res_entry * reservation);

/*
* remove the reservation at @index position in array directly. It doesn't involve semaphores (use sequentially).
* Should be used ONLY to populate array during delta loading.
* Used by load_delta() in file_op.
*/
int remove_res_from_array(unsigned int index);

#endif
