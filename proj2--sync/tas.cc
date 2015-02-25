/* this c program aims at realizing spin lock through
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


int num_th = 4;
int num_count = 10000;


int val_counter = 0;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

volatile unsigned long flag;



void init(volatile unsigned long* flag){
    flag = 0;  //0 indicate the lock is available and 1 vise versa
}

void lock(volatile unsigned long* locker){
    while (tas(locker) == 1);  //either get the lock or spin
}

void unlock(volatile unsigned long* flag){
    *flag = 0;  //reset the flag
}

void *work_thread(void *id){
	int task_id = *((int *) id);
	int i;

	for(i = 0; i < num_count; i++){
		lock(&flag);
		val_counter++;
		unlock(&flag);
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
    double hrtime1, hrtime2;

    while((i = getopt(argc,argv,"t::i::")) != -1){
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
            default:
                assert(0);
                break;
        }
    }

    /* ============ Initialize the lock ============ */    
    init(&flag);

    /* ============ Thread Creation ============ */

    id = (int *) malloc (sizeof (int) * num_th);
    tid = (pthread_t *) malloc (sizeof (pthread_t *) * num_th);
    if (!id || !tid)
        errexit ("out of shared memory");

	hrtime1 = gethrtime_x86();
	printf("Now start all the tasks, val_counter = %d\n", val_counter);


    for(i = 0; i < num_th; i++){
        id[i] = i;
        pthread_create(&tid[i], NULL, work_thread, &id[i]);
        printf("thread %d create successfully\n", i );
    }

    /* ============ Thread Join ============ */

    for(i = 0; i < num_th; i++){
        pthread_join(tid[i], NULL);
        printf("thread %d join successfully\n", i );
    }

   	hrtime2 = gethrtime_x86();
    printf("Hrtime = %f seconds, val_counter = %d\n", hrtime2 - hrtime1, val_counter);
    


    return 0;



}