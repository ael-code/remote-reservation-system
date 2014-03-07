#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "reservation.h"
#include "chiavazione.h"
#include "../lib/seats.h"

struct res_entry * array;
unsigned int array_dim;
unsigned int pwd_length;

void reservation_init(unsigned int max_rese,unsigned int pwd_l){
	//initialize chiavazione
	initialize_generator();
	pwd_length = pwd_l;
	
	void * res;
	res = calloc(max_rese,sizeof(struct res_entry));
	if(res == NULL){perror("reservation_init(): malloc");exit(-1);}
	array = (struct res_entry *)res;
	array_dim = max_rese;
	
};

char * reservation_perform(int s_num,struct seat * seats){
	//find the first hole
	int i;
	for(i=0; i<array_dim; i++){
		if(array[i].s_num ==0)
			break;
	}
	// if did not found an hole return NULL
	if(array[i].s_num !=0){
		printf("reservation_perform(): hole not found\n");
		return NULL;
	}
	
	//control if seats are available
	if(!seats_available(s_num,seats)){
		puts("seats not available");
		return NULL;
	}
	
	//occupy seats
	occupy_seats(s_num, seats);
		
	//we need to do a copy of seats because it frees after thread exit 
	void * seats_mem = malloc(sizeof(struct seat)*s_num);
	if(seats_mem==NULL){perror("reservation_perform(): malloc");exit(-1);}
	
	//arra[i].seats points to the copy of seats 
	array[i].seats = memcpy(seats_mem,seats,sizeof(struct seat)*s_num);
	
	array[i].s_num = s_num;
	array[i].chiavazione = chiavazione_gen(i,array_dim-1,pwd_length);
	
	//debug: returned chiavazione.
	puts("returned chivazione");
	return array[i].chiavazione;	
}

int reservation_delete(char * chiavazione){
	//get index in array
	unsigned int index = get_chiavazione_index(chiavazione, array_dim -1);
	
	//DEBUG:
	printf("try to delete \"%s\" at index %u\n",chiavazione,index);
	
	//return -1 if index is out of bounds
	if(index >= array_dim)return -1;
	
	//return -1 if chiavazione doesn't match
	if(array[index].chiavazione == NULL || strcmp(chiavazione,array[index].chiavazione) != 0)
		return -1;
	
	//clean seats assigned at this chiavazione
	free_seats(array[index].s_num, array[index].seats);
	
	//free chiavazione
	free(array[index].chiavazione);
	array[index].chiavazione = NULL;
	//DEBUG:
	printf("chivazione freed\n");
	
	//free seats structure
	free(array[index].seats);	
	array[index].seats = NULL;
	//DEBUG:
	printf("seats freed\n");
	
	//reset s_num
	array[index].s_num = 0;
	
	return 0;
}

struct res_entry * get_reservation(char * chiavazione){
	//get index in array
	int index = get_chiavazione_index(chiavazione, array_dim -1);
	
	//return -1 if chiavazione doesn't match
	if(array[index].chiavazione == NULL || strcmp(chiavazione,array[index].chiavazione) != 0)
		return NULL;
	
	return array+index;
}


