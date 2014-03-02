/*
*	row and coloumn of a seat
*/
struct seat{
	unsigned int row;
	unsigned int col;
};

/*
*	this is the unit that rapresent one reservation in reservation array
*/
struct res_entry{
	char * chiavazione; 	// pointer to chiavazione
	int s_num; 				// number of seat involved in this reservation
	struct seat * seats;			// pointer to an array of seats (dimension s_num)
};


void reservation_init(int max_rese,int pwd_length);
char * reservation_perform(int s_num,struct seat * seats);




