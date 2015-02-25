/*
	this program aims at the consistency model of the testing machine

	by: shaowei su 
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#define ITERATION 100000
int data;
int flag;
int turns = 0;
int arrayX[ITERATION];

int count1 = 0; // count for x = 1, y = 1;



void error_exit(const char *err_str) {
	fprintf(stderr, "%s\n", err_str);
	exit(1);
}

void *work_thread(void *thnum) {
	int task_id = *((int *) thnum); // task_id symbols which thread it is
	if (task_id == 0) {
		data = 100;
		flag = 1;
	}
	else if (task_id == 1) {
		while (flag == 0) ;
		arrayX[turns] = data;

	}
}

int main(int argc, char *argv[]){

	int i, j;
	pthread_t *tid;
	int *id;
	long r;

	int num_th = 2; // two threads are enough for this issue


	id = (int *) malloc (sizeof(int) *num_th);
	tid  = (pthread_t *) malloc (sizeof (pthread_t) *num_th);

	if(!id || !tid)
		error_exit("Out of shared memory");

	for (turns = 0; turns < ITERATION; turns++) {

		data = 0;
		flag = 0;

		for (i=0; i<num_th; i++) {
			id[i] = i;
			r = pthread_create(&tid[i], NULL, work_thread, &id[i]);
			if (r) {
				fprintf(stderr, "=====!!!Thread %d creation failed... return code from pthread_create() is %ld ...\n", i, r);
				error_exit("fail to create thread");
			}
		}
		for (i=0; i<num_th; i++) {
			r = pthread_join(tid[i], NULL);
			if (r) {
				fprintf(stderr, "=====!!!Thread %d join failed... return code from pthread_create() is %ld ...\n", i, r);
				error_exit("fail to join thread");
			}
		}
	}

	for (turns = 0; turns < ITERATION; turns++) {
		if (arrayX[turns] == 100) {
			count1 ++;
		}
	}

	printf("RESULTS:\ncount1 = %d\n", count1);

	free(id);
	free(tid);
	return 0;
}