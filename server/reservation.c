#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sem.h>
#include "reservation.h"
#include "chiavazione.h"
#include "../lib/seats.h"

static struct res_entry * array;
static struct res_entry * free_p; //pointer to the first free entry of array. It is NULL if there is no free cell
static unsigned int array_dim;
static unsigned int pwd_length;
static int semid;
static int res;

/*
*	update the pointer "free_p" searching for the next free entries.
*	If there aren't free entries it sets free_p to NULL
*/
void update_freep(){
	//should never happen
	if(free_p == NULL){puts("update_freep() find NULL pointer");exit(-1);} 
	
	//find next free entry of array
	struct res_entry * temp = free_p+1;
	while(temp-array < array_dim){
		if(temp->s_num == 0){
			free_p=temp;
			return;
		}
		temp++;
	}
	//not found any available entry
	free_p = NULL;
}

void reservation_init(unsigned int max_rese,unsigned int pwd_l){
	//initialize chiavazione generator
	initialize_generator();
	pwd_length = pwd_l;
	
	//allocate blank memory for array
	void * allocated;
	allocated = calloc(max_rese,sizeof(struct res_entry));
	if(allocated == NULL){perror("reservation_init(): malloc");exit(-1);}
	array = (struct res_entry *)allocated;
	array_dim = max_rese;
	
	//set free pointer to the first entry
	free_p = array;
	
	/*	IPC_PRIVATE isn't a flag field but a key_t type.  If this
   *   special value is used for key, the  system  call  ignores
   *   everything but the least significant 9 bits of semflg and
   *   creates a new semaphore set (on success).
	*/
	semid = semget(IPC_PRIVATE,1,IPC_CREAT|IPC_EXCL|0600);
	if(semid == -1){perror("semget in reservation_init()");exit(-1);}
	res = semctl(semid, 0, SETVAL, 1);
	if(res == -1){perror("semctl in reservation_init()");exit(-1);}
	
	
};

char * reservation_perform(int s_num,struct seat * seats){
	//prima effettua la prenotazione e poi salvala nell'array altrimenti bloccheresti tutti gli altri. 
	//Al massimo puoi controllare che siano rimasti posti liberi
	
	//if there aren't free entries return NULL
	if(free_p == NULL) return NULL;
	
	/* MATRIX ACCESS */
	
	//control if seats respect constraints
	if(control_seats(s_num, seats))return NULL;
	
	lock_seats(s_num,seats);
	
	//control if seats are free
	if(!seats_available(s_num, seats)){
		release_seats(s_num,seats);
		return NULL;
	}
		
	//occupy seats
	occupy_seats(s_num, seats);
	
	release_seats(s_num,seats);
	
	/* END MATRIX ACCESS */
	
	//paranoic control
	if(free_p == NULL){puts("I did not found free entry to store prenotation after i have occupied seats");exit(-1);}
	
	//wait until "free_p" is available
	struct sembuf sops;
	sops.sem_num = 0;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	
	res = semop(semid,&sops,sizeof(sops)/sizeof(struct sembuf));
	if(res == -1){perror("semop, waiting free pointer");exit(-1);}
	
	//save the current free_p
	struct res_entry * my_entry = free_p;
	
	//find the new free entry
	update_freep();
	
	//mark my_entry as occupied
	my_entry->s_num = s_num;
	
	//release free_p semaphore
	sops.sem_op = 1;
	res = semop(semid,&sops,sizeof(sops)/sizeof(struct sembuf));
	if(res == -1){perror("semop, waiting free pointer");exit(-1);}
		
	//we need to do a copy of seats because it frees after thread exit 
	void * seats_mem = malloc(sizeof(struct seat)*s_num);
	if(seats_mem==NULL){perror("reservation_perform(): malloc");exit(-1);}
	
	//seats in my_entry have to point to the copy of seats 
	my_entry->seats = memcpy(seats_mem,seats,sizeof(struct seat)*s_num);
	//generate chivazione
	my_entry->chiavazione = chiavazione_gen(my_entry-array,array_dim-1,pwd_length);
	
	return my_entry->chiavazione;	
}

int reservation_delete(char * chiavazione){
	//prima effettua l'accesso alla matrice e poi libera il posto della prenotazione nell'array.
	// Problemi?
	
	//get index in array
	unsigned int index = get_chiavazione_index(chiavazione, array_dim -1);
	
	//return -1 if index is out of bounds
	if(index >= array_dim)return -1;
	
	//return -1 if chiavazione doesn't match
	if(array[index].chiavazione == NULL || strcmp(chiavazione,array[index].chiavazione) != 0)
		return -1;
	
	/* MATRIX ACCESS */
	
	lock_seats(array[index].s_num, array[index].seats);
	
	free_seats(array[index].s_num, array[index].seats);
	
	release_seats(array[index].s_num, array[index].seats);
	
	/* END MATRIX ACCESS */
	
		
	//free chiavazione
	free(array[index].chiavazione);
	array[index].chiavazione = NULL;
	
	//free seats structure
	free(array[index].seats);	
	array[index].seats = NULL;
	
	//reset s_num
	array[index].s_num = 0;
	
	
	//if this entry is upper then the one pointed by free_p, update free_p
	if(free_p-array > index || free_p == NULL){
	
		//wait until free_p is available
		struct sembuf sops;
		sops.sem_num = 0;
		sops.sem_op = -1;
		sops.sem_flg = 0;
		res = semop(semid,&sops,sizeof(sops)/sizeof(struct sembuf));
		if(res == -1){perror("semop, waiting free pointer");exit(-1);}
			
		free_p = &array[index];
	
		//release free_p semaphore
		sops.sem_op = 1;
		res = semop(semid,&sops,sizeof(sops)/sizeof(struct sembuf));
		if(res == -1){perror("semop, waiting free pointer");exit(-1);}
	}
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


