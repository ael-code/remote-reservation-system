#include <stdlib.h>
#include <stdio.h>
#include "reservation.h"
#include "chiavazione.h"

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
	if(array[i].s_num !=0){printf("reservation_perform(): hole not found");exit(-1);}
	
	array[i].s_num = s_num;
	array[i].chiavazione = chiavazione_gen(i,array_dim-1,pwd_length);
	array[i].seats = seats;
	
	return array[i].chiavazione;	
}

int reservation_delete(char * chiavazione){
	//get index in array
	int index = get_chiavazione_index(chiavazione, array_dim -1);
	
	//return -1 if chiavazione doesn't match
	if(array[index].chiavazione == NULL || strcmp(chiavazione,array[index].chiavazione) != 0)
		return -1;
		
	free((void *)array[index].chiavazione);
	array[index].chiavazione = NULL;
	free((void *)array[index].seats);
	array[index].seats = NULL;
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


