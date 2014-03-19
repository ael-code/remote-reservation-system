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
		if(pthread_equal (punt->next->ptid,tid)){
			punt->next = punt->next->next;
			
			#ifdef DEBUG
			char name[22];
			pthread_getname_np(pthread_self(),name,sizeof(name));
			printf("deleted thread: \"%s\" from threads-struct\n", name);
			#endif
			
			break;
		}
	}
	head = fake_head.next;
}

void kill_all_threads(){
	#ifdef DEBUG
	char name[22];
	pthread_getname_np(pthread_self(),name,sizeof(name));
	printf("thread: \"%s\" is killing all other thread\n",name);
	#endif
	
	struct pthread_node * punt = head;
	while(punt != NULL){
		if(!pthread_equal(punt->ptid,pthread_self())){
			#ifdef DEBUG
			pthread_getname_np(punt->ptid,name,sizeof(name));
			printf("killing thread: \"%s\"\n",name);
			#endif
			if(pthread_cancel(punt->ptid))
				puts("error closing thread");
			pthread_join(punt->ptid,NULL);
		}
		struct pthread_node * temp = punt;
		punt = punt->next;
		free(temp);
	}
}

