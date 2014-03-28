#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sem.h>
#include <fcntl.h>
#include "matrix.h"
#include "file_op.h"
#include "reservation.h"
#include "chiavazione.h"
#include "seats.h"

struct res_entry * reserv_array;
static struct res_entry * free_p; //pointer to the first free entry of array. It is NULL if there is no free cell
static unsigned int array_dim;
static unsigned int pwd_length;
static int semid;
static int res;

void update_freep(unsigned int index){
	
	//paranoic control
	if(index >= array_dim){puts("reservation.c: index out of bounds in update_freep()");exit(-1);}
	
	//case entry was freed
	if((reserv_array+index)->s_num == 0){
		//if this entry is upper then the one pointed by free_p, update free_p
		if(free_p == NULL || free_p-reserv_array > index )		
			free_p = reserv_array+index;
			
	//case entry was occupied
	}else{
		//find next free entry of reserv_array
		struct res_entry * temp = free_p+1;
		while(temp-reserv_array < array_dim){
			if(temp->s_num == 0){
				free_p=temp;
				return;
			}
			temp++;
		}
		//not found any available entry
		free_p = NULL;
	}
	
}

void reservation_close(){
	res = semctl(semid,0,IPC_RMID);
	if(res == -1){perror("deleting semid in reservation.c");}
}

void reservation_init(unsigned int max_rese,unsigned int pwd_l){
	//initialize chiavazione generator
	initialize_generator();
	pwd_length = pwd_l;
	
	//allocate blank memory for reserv_array
	void * allocated;
	allocated = calloc(max_rese,sizeof(struct res_entry));
	if(allocated == NULL){perror("reservation_init(): malloc");exit(-1);}
	reserv_array = (struct res_entry *)allocated;
	array_dim = max_rese;
	
	//set free pointer to the first entry
	free_p = reserv_array;
	
	/*	IPC_PRIVATE isn't a flag field but a key_t type.  If this
   *   special value is used for key, the  system  call  ignores
   *   everything but the least significant 9 bits of semflg and
   *   creates a new semaphore set (on success).
	*/
	semid = semget(IPC_PRIVATE,1,0600);
	if(semid == -1){perror("semget in reservation_init()");exit(-1);}
	res = semctl(semid, 0, SETVAL, 1);
	if(res == -1){perror("semctl in reservation_init()");exit(-1);}
	
	
}

char * reservation_perform(int s_num,struct seat * seats){
	//prima effettua la prenotazione e poi salvala nell'array altrimenti bloccheresti tutti gli altri. 
	//Al massimo puoi controllare che siano rimasti posti liberi
	
	//if there aren't free entries return NULL
	if(free_p == NULL) return NULL;
	
	//control if seats respect constraints
	if(control_seats(s_num, seats))return NULL;
	
	/* MATRIX ACCESS */
	wait_master_semaphore();
	
	int lock_res = lock_seats(s_num, seats);
	if(lock_res) exclusive_lock_seats(s_num, seats);
	
	//control if seats are free
	if(!seats_available(s_num, seats)){
		if(lock_res) exclusive_release_seats(s_num, seats);
		else release_seats(s_num, seats);
		return NULL;
	}
		
	//occupy seats
	occupy_seats(s_num, seats);
	
	if(lock_res) exclusive_release_seats(s_num, seats);
	else release_seats(s_num, seats);
	
	/* END MATRIX ACCESS */
	
	//paranoic control
	if(free_p == NULL){puts("I did not found free entry to store prenotation after i have occupied seats");exit(-1);}
	
	/* FREE POINTER ACCESS */
	
	//wait until "free_p" is available
	struct sembuf sops;
	sops.sem_num = 0;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	
	res = semop(semid,&sops,sizeof(sops)/sizeof(struct sembuf));
	if(res == -1){perror("semop, waiting free pointer");exit(-1);}
	
	//save the current free_p
	struct res_entry * my_entry = free_p;
	
	//mark my_entry as occupied
	my_entry->s_num = s_num;
	
	//find the new free entry
	update_freep(free_p-reserv_array);
	
	//release free_p semaphore
	sops.sem_op = 1;
	res = semop(semid,&sops,sizeof(sops)/sizeof(struct sembuf));
	if(res == -1){perror("semop, waiting free pointer");exit(-1);}
	
	/* END POINTER ACCESS */
		
	//we need to do a copy of seats because it frees after thread exit 
	void * seats_mem = malloc(sizeof(struct seat)*s_num);
	if(seats_mem==NULL){perror("reservation_perform(): malloc");exit(-1);}
	
	//seats in my_entry have to point to the copy of seats 
	my_entry->seats = memcpy(seats_mem,seats,sizeof(struct seat)*s_num);
	//generate chivazione
	my_entry->chiavazione = chiavazione_gen(my_entry-reserv_array,array_dim-1,pwd_length);
	
	//save delta on file
	if(save_delta_add(my_entry - reserv_array, my_entry)){
		puts("reservation.c: error on save_delta_add()");
		exit(-1);
	}
	
	return my_entry->chiavazione;	
}

int reservation_delete(char * chiavazione){
	//prima effettua accesso array prenotazioni e poi a matrice dei posti
	
	//get index in array
	unsigned int index = get_chiavazione_index(chiavazione, array_dim -1);
	
	//return -1 if index is out of bounds
	if(index >= array_dim)return -1;
	
	//return -1 if chiavazione doesn't match
	if(reserv_array[index].chiavazione == NULL || strcmp(chiavazione,reserv_array[index].chiavazione) != 0)
		return -1;
	
	//Store res_entry before cleaning it
	char * temp_chiavazione = reserv_array[index].chiavazione;
	reserv_array[index].chiavazione = NULL;
	
	struct seat * temp_seats = reserv_array[index].seats;
	reserv_array[index].seats = NULL;

	int temp_s_num = reserv_array[index].s_num;
	
	
	/* FREE POINTER ACCESS */
	
	//wait until free_p is free & lock free_p
	struct sembuf sops;
	sops.sem_num = 0;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	res = semop(semid,&sops,sizeof(sops)/sizeof(struct sembuf));
	if(res == -1){perror("semop, waiting free pointer");exit(-1);}
	
	/*	critic! after this operation this res_entry result free
	* 	can't stay before pointer access cause could become occupied 
	* 	before updating free pointer for this entry
	*/
	reserv_array[index].s_num = 0;
	
	update_freep(index);
	
	//release free_p semaphore
	sops.sem_op = 1;
	res = semop(semid,&sops,sizeof(sops)/sizeof(struct sembuf));
	if(res == -1){perror("semop, waiting free pointer");exit(-1);}
	
	/* END FREE POINTER ACCESS */
	
	/* MATRIX ACCESS */
	
	free_seats(temp_s_num, temp_seats);
	
	/* END MATRIX ACCESS */
	
	//save delta del
	if(save_delta_del(index)){
		puts("reservation.c: save_delta_del()");
		exit(-1);
	}
	
	//free chiavazione
	free(temp_chiavazione);
	
	//free seats structure
	free(temp_seats);
	
	return 0;
}

struct res_entry * get_reservation(char * chiavazione){
	//get index in array
	int index = get_chiavazione_index(chiavazione, array_dim -1);
	
	//return -1 if chiavazione doesn't match
	if(reserv_array[index].chiavazione == NULL || strcmp(chiavazione,reserv_array[index].chiavazione) != 0)
		return NULL;
	
	return reserv_array+index;
}

/*
* insert this @reservation in array directly. It doesn't involve semaphores (use sequentially).
* Should be used ONLY to populate array during delta loading.
* Used by load_delta() in file_op.
*/
int insert_res_in_array(unsigned int index, struct res_entry * reservation){
	//return -1 if index is out of bounds
	if(index >= array_dim)return -1;
	
	//paranoic control
	if(free_p == NULL || free_p != reserv_array+index) return -1;
	
	//control if seats respect constraints
	if(control_seats(reservation->s_num,reservation->seats))return -1;
	
	occupy_seats(reservation->s_num, reservation->seats);
	
	//fill in res_entry
	reserv_array[index].s_num = reservation->s_num;
	reserv_array[index].seats = reservation->seats;
	reserv_array[index].chiavazione = reservation->chiavazione;

	//find the new free entry
	update_freep(index);
	
	return 0;
}

/*
* remove the reservation at @index position in array directly. It doesn't involve semaphores (use sequentially).
* Should be used ONLY to populate array during delta loading.
* Used by load_delta() in file_op.
*/
int remove_res_from_array(unsigned int index){
	//return -1 if index is out of bounds
	if(index >= array_dim)return -1;
	
	free_seats(reserv_array[index].s_num, reserv_array[index].seats);
	
	//clean res_entry
	reserv_array[index].s_num = 0;
	free(reserv_array[index].chiavazione);
	reserv_array[index].chiavazione = NULL;
	free(reserv_array[index].seats);
	reserv_array[index].seats = NULL;
	
	update_freep(index);
		
	return 0;
}

