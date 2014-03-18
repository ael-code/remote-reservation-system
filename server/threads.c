#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "threads.h"
static struct pthread_node * head;

void add_thread(pthread_t tid){
	struct pthread_node * new_node =malloc(sizeof(struct pthread_node));
	if(new_node == NULL){perror("malloc add_thread");exit(-1);}
	
	new_node->ptid = tid;
	new_node->next = head;
	
	head = new_node;	
}

void del_thread(pthread_t tid){
	struct pthread_node fake_head;
	fake_head.next = head;
	
	struct pthread_node * punt = &fake_head;
	
	while(punt->next != NULL){
		if(punt->next->ptid == tid){
			punt->next = punt->next->next;
		}
	}
	head = fake_head.next;
}

void kill_all_threads(){
	struct pthread_node * punt = head;
	while(punt != NULL){
		if(punt->ptid != pthread_self()){
			if(pthread_cancel(punt->ptid))
				puts("error closing thread");
			pthread_join(punt->ptid,NULL);
		}
		struct pthread_node * temp = punt;
		punt = punt->next;
		free(temp);
	}
}

