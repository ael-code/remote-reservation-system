#ifndef __RESERVATION__
#define __RESERVATION__

/*
*	this is the unit that rapresent one reservation in reservation array
*/
struct res_entry{
	char * chiavazione; 		// pointer to chiavazione
	int s_num; 					// number of seat involved in this reservation
	struct seat * seats;		// pointer to an array of seats (dimension s_num)
};

void reservation_close();

void reservation_init(unsigned int max_rese,unsigned int pwd_length);

char * reservation_perform(int s_num,struct seat * seats);

int reservation_delete(char * chiavazione);

struct res_entry * get_reservation(char * chiavazione);

#endif
