#include <stdlib.h>
#include <stdio.h>
#include "reservation.h"
#include "chiavazione.h"

struct res_entry * array;
int array_dim;
int pwd_length;

void reservation_init(int max_rese,int pwd_l){
	
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
	
	array[i].chiavazione = chiavazione_gen(i,array_dim,pwd_length);
	array[i].s_num = s_num;
	array[i].seats = seats;
	
	return array[i].chiavazione;	
}











