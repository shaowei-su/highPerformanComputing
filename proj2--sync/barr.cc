/* this cpp program aims at realizing barrier through
 * different algorithm  -- shaowei su
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
 
#include "hrtimer_x86.h"
#include "atomic_ops.h"

#define MAX_THREADS 128



int num_th = 4;  //the number of threads launched, default value would be 4
int num_count = 10000;  //the number of counts per thread, default as 10000
int lock_mode = 1;  //indicates the type of locks, default as no synchronization


pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/* code snippet from sor_pthread.c */
void barrier1(int expect)
{
  static int arrived = 0;

  pthread_mutex_lock (&mut);    //lock

  arrived++;
  if (arrived < expect)
    pthread_cond_wait (&cond, &mut);
  else {
    arrived = 0;        // reset the barrier before broadcast is important
    pthread_cond_broadcast (&cond);
  }

  pthread_mutex_unlock (&mut);  //unlock
}
/* code snippet from http://www.cs.rochester.edu/u/sandhya/csc458/assignments/proj2/barrier.c */
void barrier2(int tid)
{
    static volatile unsigned long count = 0;
    static volatile unsigned int sense = 0;
    static volatile unsigned int thread_sense[MAX_THREADS] = {0};

    thread_sense[tid] = !thread_sense[tid];
    if (fai(&count) == num_th-1) {
        count = 0;
        sense = !sense;
    } else {
        while (sense != thread_sense[tid]);     /* spin */
    }
}



void *work_thread1(void *id){
	int task_id = *((int *) id);
	int i;
    double hrtime1, hrtime2;
    barrier1(num_th);
    if (task_id == 0) {
        hrtime1 = gethrtime_x86();
        printf("Now start all the tasks with PTHREADS_BARRIER\n");;
    }
    barrier1(num_th);
	for (i = 0; i < num_count; i++) {
		barrier1(num_th);
	}
    barrier1(num_th);
    if (task_id == 0) {
        hrtime2 = gethrtime_x86();
        printf("Hrtime = %f seconds\n", hrtime2 - hrtime1);
    
    }
    return NULL;
}

void *work_thread2(void *id){
    int task_id = *((int *) id);
    int i;
    double hrtime1, hrtime2;
    barrier2(task_id);
    if (task_id == 0) {
        hrtime1 = gethrtime_x86();
        printf("Now start all the tasks with CENTRALIZED_SENSE_REVERSING_BARRIER\n");;
    }
    barrier2(task_id);
    for (i = 0; i < num_count; i++) {
        barrier2(task_id);
    }
    barrier2(task_id);
    if (task_id == 0) {
        hrtime2 = gethrtime_x86();
        printf("Hrtime = %f seconds\n", hrtime2 - hrtime1);
    
    }
    return NULL;
}


extern char * optarg;
/* code snippet from sor_thread.c */
void
errexit (const char *err_str)
{
  fprintf (stderr, "%s", err_str);
  exit (1);
}

int main(int argc, char *argv[]){
	int i;
	pthread_t *tid;
	int *id;


    while((i = getopt(argc,argv,"t:i:m:")) != -1){
        switch(i){
            case 't':
                {
                    int t;
                    t = atoi(optarg);
                    if (t > 0){
                        num_th = t;
                    } else {
                        fprintf(stderr,"Entered size is negative, hence using the default value: 4\n");
                    }
                }
                break;
            case 'i':
                {
                    int i;
                    i = atoi(optarg);
                    if (i > 0){
                        num_count = i;
                    } else {
                        fprintf(stderr,"Entered task number is negative, hence using the default value: 10000\n");
                    }
                }
                break;
            case 'm':
                {
                    int m;
                    m = atoi(optarg);
                    if ((m > 0) && (m < 4)){
                        lock_mode = m;
                    } else {
                        fprintf(stderr,"Please confirm your selection of lock mode, default will be no synchronization:\n1 : Pthread-based barrier; \n2 : Centralized sense-reversing barrier;\n3 : Tree-based barrier;\n");
                    }
                }
                break;
            default:
                assert(0);
                break;
        }
    }

    /* ============ Thread Creation ============ */

    id = (int *) malloc (sizeof (int) * num_th);
    tid = (pthread_t *) malloc (sizeof (pthread_t *) * num_th);
    if (!id || !tid)
        errexit ("out of shared memory");


    for(i = 0; i < num_th; i++){
        id[i] = i;
        switch(lock_mode){
        	case 1: pthread_create(&tid[i], NULL, work_thread1, &id[i]); break;
            case 2: pthread_create(&tid[i], NULL, work_thread2, &id[i]); break;
        	default: errexit ("Lock mode error\n");
        }
    }

    /* ============ Thread Join ============ */

    for(i = 0; i < num_th; i++){
        pthread_join(tid[i], NULL);
    }


    return 0;



}