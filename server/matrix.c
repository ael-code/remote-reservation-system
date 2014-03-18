#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/sem.h>
#include <errno.h>

#include "matrix.h"

#define T_OUT 4
 
static char * mat;
static unsigned int rows;
static unsigned int cols;
static int semid;
static int sem_timeout;
static int res;


int semtimedop(int,struct sembuf*, unsigned, struct timespec *);

void matrix_close(){
	res = semctl(semid,0,IPC_RMID);
	if(res == -1){perror("deleting semid in matrix.c");}
	res = semctl(sem_timeout,0,IPC_RMID);
	if(res == -1){perror("deleting sem_timeout in matrix.c");}
	//free(mat);
}
/*
*	Allocate memory and set dimension for mat
*/
void matrix_init(unsigned int mat_rows, unsigned int mat_cols){
	
	//set dimensions
	rows = mat_rows;
	cols = mat_cols;
	
	//allocate memory for mat
	mat = (char * )calloc(sizeof(char),rows*cols);
	if(mat == NULL){printf("ERROR: calloc");exit(-1);}
	
	/*	IPC_PRIVATE isn't a flag field but a key_t type.  If this
   *   special value is used for key, the  system  call  ignores
   *   everything but the least significant 9 bits of semflg and
   *   creates a new semaphore set (on success).
	*/
	semid = semget(IPC_PRIVATE,rows*cols,0600);
	if(semid == -1){perror("semget in matrix_init()");exit(-1);}
	
	unsigned short vals[rows*cols];
	int i;
	for(i = 0;i<rows*cols;i++)
		vals[i]=1;
	
	res = semctl(semid, rows*cols, SETALL, vals);
	if(res == -1){perror("semctl in matrix_init()");exit(-1);}

	sem_timeout = semget(IPC_PRIVATE,2,0600);
	if(sem_timeout == -1){perror("semget in matrix_init()");exit(-1);}
	
	res = semctl(sem_timeout, 0, SETVAL, 0);
	if(res == -1){perror("semctl in matrix_init()");exit(-1);}
	
	res = semctl(sem_timeout, 1, SETVAL, 1);
	if(res == -1){perror("semctl in matrix_init()");exit(-1);}
	
}

/*
*	return pointer to the matrix
*/
char * get_matrix(){
	return mat;
}

/*
*	return 1 if all cells indicated by "seats" are free. (no input controls)
*/
int seats_available(unsigned int num, struct seat * seats){
	char (*matrix)[cols] =(char (*)[cols]) mat;
	struct seat * punt = seats;
	while( (punt-seats) < num ){
		if( matrix[punt->row][punt->col] != 0)
			return 0;
		punt++;
	}
	return 1;
}

/*  without the cast
		if( *(mat+(punt->row*cols)+punt->col) != 0)
*/

/*
*	Set to 1 all cells indicated by "seats". (no input controls)
*/
void occupy_seats(unsigned int num, struct seat * seats){
	char (*matrix)[cols] =(char (*)[cols]) mat;
	struct seat * punt = seats;
	while( (punt-seats) < num ){
		//paranoic control
		if(matrix[punt->row][punt->col] == 1){puts("found seats already occupied");exit(-1);}
		matrix[punt->row][punt->col] = 1;
		punt++;
	}
}

/*
*	Set to 0 all cells indicated by "seats". (no input controls)
*/
void free_seats(unsigned int num, struct seat * seats){
	char (*matrix)[cols] =(char (*)[cols]) mat;
	struct seat * punt = seats;
	while( (punt-seats) < num ){
		//paranoic control
		if(matrix[punt->row][punt->col] == 0){puts("found seats already free");exit(-1);}
		matrix[punt->row][punt->col] = 0;
		punt++;
	}
}

/*
*	Try to lock semaphores on these "seats". 
* 	This operation is blocking for T_OUT time, after which it returns -1.
*  Prevent Starvation
*/
int lock_seats(unsigned int num, struct seat * seats){
	struct seat * punt = seats;
	struct sembuf sops[num];
	int i;
	for(i = 0; i<num; i++){
		sops[i].sem_num = ( punt[i].row * cols ) + punt[i].col;
		sops[i].sem_op = -1;
		sops[i].sem_flg = SEM_UNDO;	
	}
	
	struct timespec time_s;
	time_s.tv_sec = T_OUT; //time_t
	time_s.tv_nsec = 0; //long
	res = semtimedop(semid,sops,sizeof(sops)/sizeof(struct sembuf), &time_s);
	if(res == -1){
		if(errno == EAGAIN){
			return -1;
		}else{
			perror("semop, locking matrix semaphores");
			exit(-1);
		}
	}
	return 0;
}

/*
*	Locks semaphores on these "seats" in a special way.
* 	Should be called only after a lock_seats() time-out.
*
* 		- Increase master semaphore (temporary deny access to other non stucked threads)
*		- Wait for my turn on time-out semaphore (temporary deny access to other stucked threads)
*		- Locks semaphores on these "seats".
*/
void exclusive_lock_seats(unsigned int num, struct seat * seats){
	//prepare lock structure
	struct seat * punt = seats;
	struct sembuf sops[num];
	int i;
	for(i = 0; i<num; i++){
		sops[i].sem_num = ( punt[i].row * cols ) + punt[i].col;
		sops[i].sem_op = -1;
		sops[i].sem_flg = SEM_UNDO;	
	}
	
	//deny access to the matrix for the other threads
	struct sembuf sop_timeout;
	sop_timeout.sem_num = 0;
	sop_timeout.sem_op = 1;
	sop_timeout.sem_flg = SEM_UNDO;
	res = semop(sem_timeout,&sop_timeout,1);
	if(res == -1){perror("semop increasing sem_timeout[0]");exit(-1);}
	
	//waiting for other stucked threads
	sop_timeout.sem_num = 1;
	sop_timeout.sem_op = -1;
	res = semop(sem_timeout,&sop_timeout,1);
	if(res == -1){perror("semop waiting sem_timeout[1]");exit(-1);}
	
	//proceed with sops
	res = semop(semid,sops,sizeof(sops)/sizeof(struct sembuf));
	if(res == -1){perror("semop, special locking");exit(-1);}
}

/*
*	Release semaphores on these "seats". (no input controls)
*/
void release_seats(unsigned int num, struct seat * seats){
	struct seat * punt = seats;
	struct sembuf sops[num];
	int i;
	for(i = 0; i<num; i++){
		sops[i].sem_num = ( punt[i].row * cols ) + punt[i].col ;
		sops[i].sem_op = 1;
		sops[i].sem_flg = 0;	
	}
	res = semop(semid,sops,sizeof(sops)/sizeof(struct sembuf));
	if(res == -1){perror("semop, releasing matrix sempaphores");exit(-1);}
}
/*
*	Release semaphores on these "seats" in a special way.
* 	Should be called only after a special_lock_seats().
*
*		- Release semaphores on these "seats".
*		- Restore to 1 time-out semaphore (allow access to others stucked threads)
* 		- Increase master semaphore (allow access to other non stucked threads)
*/
void exclusive_release_seats(unsigned int num, struct seat * seats){
	//release matrix semaphores
	struct seat * punt = seats;
	struct sembuf sops[num];
	int i;
	for(i = 0; i<num; i++){
		sops[i].sem_num = ( punt[i].row * cols ) + punt[i].col ;
		sops[i].sem_op = 1;
		sops[i].sem_flg = 0;	
	}
	res = semop(semid,sops,sizeof(sops)/sizeof(struct sembuf));
	if(res == -1){perror("semop, releasing matrix sempaphores");exit(-1);}
	
	//releasing other stucked threads
	struct sembuf sop_timeout;
	sop_timeout.sem_num = 1;
	sop_timeout.sem_op = 1;
	sop_timeout.sem_flg = 0;
	res = semop(sem_timeout,&sop_timeout,1);
	if(res == -1){perror("semop releasing sem_timeout[1]");exit(-1);}
	
	//allow access to the matrix for the other threads
	sop_timeout.sem_num = 0;
	sop_timeout.sem_op = -1;
	res = semop(sem_timeout,&sop_timeout,1);
	if(res == -1){perror("semop releasing sem_timeout[0]");exit(-1);}
	
}

void wait_master_semaphore(){
	struct sembuf sop;
	sop.sem_num = 0;
	sop.sem_op = 0;
	sop.sem_flg = 0;
	res = semop(sem_timeout,&sop,1);
	if(res == -1){perror("semop, wait_master_semaphore");exit(-1);}
}


/*
*	If these "seats" violate constraints return (int > 0).
* 	# 1 if parameter aren't good
*	# 2 if the seats are out og bounds
* 	# 3 if seats array contains duplicate
*/
int control_seats(unsigned int num, struct seat * seats){
	//check parameter
	if(num < 1 || seats == NULL)
		return 1;
	
	struct seat * punt;
	struct seat * puntB;
	
	//check bounds
	punt = seats;
	while( (punt-seats) < num ){
		if(punt->row >= rows || punt -> col >= cols)
			return 2;
		punt++;
	}
	
	//check duplicate
	punt = seats;
	while((punt-seats)< num-1){
		puntB = punt+1;
		while((puntB-seats)<num){
			if(punt->row == puntB->row && punt->col == puntB->col)
				return 3;
			puntB++;
		}
		punt++;
	}
	
	return 0;
}

/* DEBUG
void printSem(char * msg){
	printf("%s\n",msg);	
	
	struct semid_ds sds;
	res = semctl(semid, 0,IPC_STAT,&sds);
	if(res == -1){perror("semctl in matrix_init()");exit(-1);}	
	printf("sem dim %ld\n",sds.sem_nsems);
	
	int i;
	for(i=0;i<rows*cols;i++)printf("sem[%d]= %d\n",i,semctl(semid, i, GETVAL));
}
*/
