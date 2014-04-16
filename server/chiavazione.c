#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "chiavazione.h"
#include "conversion.h"

static char * seed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/*
* Generate a chiavazione with index "index" 
* and a password length of "pwd_length"
*/
char * chiavazione_gen(unsigned int index, unsigned int max_index ,unsigned int pwd_length){
	pwd_length++; // include '\0'
	int seed_length = strlen(seed);
	unsigned int i;
	unsigned int index_max_length = int_to_charc(max_index);
	
	char * result;
	
	result = (char* )malloc(index_max_length+pwd_length);
	if(result == NULL){perror("malloc"); return NULL;}
	
	char temp_i[index_max_length];
	sprintf(temp_i,"%d",index);
	
	int zeros = index_max_length - strlen(temp_i);
	char * punt = result;
	
	for(i=0; i<zeros; i++){
		*punt++ = '0';
	}

	for(i=0; i<strlen(temp_i); i++){
		*punt++ = temp_i[i];
	}
	
	for( i=0; i<pwd_length-1; i++){
		int random = rand()%seed_length;		
		*punt++ = seed[random];
	}
	*punt='\0';
	
	return result;
}

/*
* Return an int corresponding to the index of "chiavazione"
*/
unsigned int get_chiavazione_index(char * chiavazione, unsigned int max_index){
	int index_max_length = int_to_charc(max_index);
	char tbuff[index_max_length+1];
	int i;
	for(i=0; i<index_max_length; i++){
		tbuff[i] = chiavazione[i];
	}
	tbuff[i]='\0';
	return strtol(tbuff,NULL,10); //cast from long to u_int
}

/*
*	Return the length for a chiavazione 
*	whith this max_inex and this pwd length
*    (doesn't include '\0')
*/
unsigned int get_chiavazione_length(unsigned int max_index,unsigned int pwd_length){
	return int_to_charc(max_index)+pwd_length;
}

unsigned int get_seed_length(){
	return strlen(seed);
}

/*
* Needed to initialize seed for rand() function
*/
void initialize_generator(){
	srand(time(NULL));
}
