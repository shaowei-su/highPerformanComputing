/* this cpp program aims at realizing barrier through
 * different algorithm  -- shaowei su
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <xmmintrin.h>
#include "hrtimer_x86.h"
#include "atomic_ops.h"

#define MAX_THREADS 128

typedef struct _node
{
    volatile unsigned long count;
    int sense;
    volatile unsigned long k;
    struct _node *parent;
} node;


typedef struct _barr
{
    node *leaf[MAX_THREADS];
} barr;

int num_th = 4;  //the number of threads launched, default value would be 4
int num_count = 10000;  //the number of counts per thread, default as 10000
int lock_mode = 1;  //indicates the type of locks, default as no synchronization
int r = 2;
barr *B;

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

/*
    Initialize the root with count as 2, then build the rest of the tree:
*/
/*
    In order to build a balanced tree, we start from the last thread, with thread ID: num_th - 1;
    Then we attach it the the location 2 * (num_th - 1)
    The following threads go backward until all attached to the tree.

    All of the node attached by a thread have node->count = 1;
    The rest of previous nodes(including root) have node->count = 2;

    After the initialize of count, we save it to node->k for reuse.
*/
void InitializeBarrier(barr *B) {
    int i;
    node* root = B->leaf[0];
    root->count = 2; 
    root->parent = NULL;
    root->sense = 0;

    for (i = (2 * (num_th - 1)); i >= (num_th - 1); i--) {
        B->leaf[i]->count = 1;
        B->leaf[i]->parent = B->leaf[(i-1)/2];
        B->leaf[i]->sense = 0;
    }
    
    for(i = 1; i < (num_th - 1); i++){
        B->leaf[i]->count = 2;
        B->leaf[i]->sense = 0;
        B->leaf[i]->parent = B->leaf[(i-1)/2];
    }

    for (i = 0; i < num_th; i++){
        B->leaf[i]->k = B->leaf[i]->count;
    }
}

/*
    Spin if the count is larger than 0;
    else, call the wait recuisively on parent
*/

void wait(node *nd, int* mysense) {

    int count_cur;

    count_cur = faa(&nd->count,-1);
    if (count_cur == 1) {
        if (nd->parent != NULL) {
        	wait(nd->parent, mysense);
        }
        nd->count = nd->k;
        nd->sense = *mysense;
    }
    else{
        while (nd->sense != *mysense){}
    }
}
/*
    Call wait() to reduce parent->count by 1;
    Then, reverse the value of mysense for reuse;
*/
void await (barr *B, int i, int* mysense) {

    wait(B->leaf[num_th-1+i]->parent, mysense);
    *mysense = 1 - *mysense;
}

void *work_thread1(void *id){
	int task_id = *((int *) id);
	int i;
    double hrtime1, hrtime2;
    barrier1(num_th);
    pthread_t  thread_id;
    cpu_set_t cpuset;
    int setted, set_aff;
    setted = task_id % num_cpu;
    thread_id = pthread_self(); // get the ID of calling thread
    CPU_ZERO(&cpuset);
    CPU_SET(setted, &cpuset); // set specified CPU in a set

    set_aff = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset); // set thread I to core I
    assert(set_aff == 0);
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
    pthread_t  thread_id;
    cpu_set_t cpuset;
    int setted, set_aff;
    setted = task_id % num_cpu;
    thread_id = pthread_self(); // get the ID of calling thread
    CPU_ZERO(&cpuset);
    CPU_SET(setted, &cpuset); // set specified CPU in a set

    set_aff = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset); // set thread I to core I
    assert(set_aff == 0);
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

void *work_thread3(void *id){
    int task_id = *((int *) id);
    int i;
    double hrtime1, hrtime2;
    int mysense = 1;
    await(B, task_id, &mysense);
    pthread_t  thread_id;
    cpu_set_t cpuset;
    int setted, set_aff;
    setted = task_id % num_cpu;
    thread_id = pthread_self(); // get the ID of calling thread
    CPU_ZERO(&cpuset);
    CPU_SET(setted, &cpuset); // set specified CPU in a set

    set_aff = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset); // set thread I to core I
    assert(set_aff == 0);
    if (task_id == 0) {
        hrtime1 = gethrtime_x86();
        printf("Now start all the tasks with TREE_BARRIER\n");;
    }

    await(B, task_id, &mysense);
    
    for (i = 0; i < num_count; i++) {
        await(B, task_id, &mysense);
    }
    
    await(B, task_id, &mysense);

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
    /* ============ Initialization of combining tree barrier ============ */
    B = (barr*) malloc(sizeof(barr));
    for(i=0; i<MAX_THREADS; i++){
    	B->leaf[i] = (node *) malloc(sizeof(node));
    }
    InitializeBarrier(B);
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
            case 3: pthread_create(&tid[i], NULL, work_thread3, &id[i]); break;
        	default: errexit ("Lock mode error\n");
        }
    }

    /* ============ Thread Join ============ */

    for(i = 0; i < num_th; i++){
        pthread_join(tid[i], NULL);
    }

    for(i=0; i<MAX_THREADS; i++){
    	free(B->leaf[i]);
    }

    free(B);

    return 0;

}