#ifndef __CHIAVAZIONE__
#define __CHIAVAZIONE__
/*
* Generate a chiavazione with index "index" 
* and a password length of "pwd_length"
*/
char * chiavazione_gen(unsigned int index, unsigned int	max_index ,unsigned int pwd_length);

/*
* Return an int corresponding to the index of "chiavazione"
*/
unsigned int get_chiavazione_index(char * chiavazione, unsigned int max_index);

/*
*	Return the length for a chiavazione 
*	whith this max_inex and this pwd length
*/
unsigned int get_chiavazione_length(unsigned int max_index,unsigned int pwd_length);

/*
* Needed to initialize seed for rand() function
*/
void initialize_generator();

#endif
