#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static char * seed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

// generazione chiave prenotazione (chiavzione)
char * chiavazione_gen(unsigned int index, unsigned int	max_index ,unsigned int pwd_length){
	int length = strlen(seed);
	unsigned int i;
	char pwd[pwd_length];
	int index_max_length = (int) ceil(log10((double)max_index+1));
	
	char * result;
	
	result = (char* )malloc(4+pwd_length);
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
		int random = rand()%length;		
		*punt++ = seed[random];
	}
	*punt='\0';
	
	return result;
}

void initialize_generator(){
	srand(time(NULL));
}
