#ifndef __THREADS__
#define __THREADS__

struct pthread_node{
	pthread_t ptid;
	struct pthread_node * next;
};

void add_thread(pthread_t tid);

void del_thread(pthread_t tid);

void kill_all_threads();

#endif
