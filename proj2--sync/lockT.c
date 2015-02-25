/* this cpp program aims at realizing spin lock through
 * different algorithm  -- shaowei su
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <sched.h>
#include <math.h>
#include "hrtimer_x86.h"
#include "atomic_ops.h"

#define MAX_THREADS 128
int num_th = 4;  //the number of threads launched, default value would be 4
int num_count = 10000;  //the number of counts per thread, default as 10000
int lock_mode = 2;  //indicates the type of locks, default as no synchronization
int num_cpu = 32;
volatile unsigned long val_counter = 0;  //indicate the current value of counter
volatile unsigned long flag;  //used for tas/tatas locks
ticket_lock_t tick;  //used for ticket locks
mcs_qnode_t* L;  //queue of mcs lock nodes

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;


void barrier(int tid)
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
    barrier(task_id);
    pthread_t  thread_id;
    cpu_set_t cpuset;
    int set_aff;

    thread_id = pthread_self(); // get the ID of calling thread
    CPU_ZERO(&cpuset);
    CPU_SET(task_id, &cpuset); // set specified CPU in a set

    set_aff = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset); // set thread I to core I
    assert(set_aff == 0);


    if (task_id == 0) {
        val_counter = 0;
        hrtime1 = gethrtime_x86();
        printf("Now start all the tasks with NO_LOCK, val_counter = %lu\n", val_counter);
    }
    barrier(task_id);
	for (i = 0; i < num_count; i++) {
		val_counter++;
	}
    barrier(task_id);
    if (task_id == 0) {
        hrtime2 = gethrtime_x86();
        printf("Hrtime = %f seconds, val_counter = %lu\n", hrtime2 - hrtime1, val_counter);
    
    }
    return NULL;
}

void *work_thread2(void *id){
	int task_id = *((int *) id);
	int i;
    double hrtime1, hrtime2;
    barrier(task_id);
    pthread_t  thread_id;
    cpu_set_t cpuset;
    int set_aff;

    thread_id = pthread_self(); // get the ID of calling thread
    CPU_ZERO(&cpuset);
    CPU_SET(task_id, &cpuset); // set specified CPU in a set

    set_aff = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset); // set thread I to core I
    assert(set_aff == 0);



    if (task_id == 0) {
        val_counter = 0;
        hrtime1 = gethrtime_x86();
        printf("Now start all the tasks with PTHREAD_LOCK, val_counter = %lu\n", val_counter);
    }
    barrier(task_id);
	for(i = 0; i < num_count; i++){
		pthread_mutex_lock (&mut);
		val_counter++;
		pthread_mutex_unlock (&mut);
	}
    barrier(task_id);
    if (task_id == 0) {
        hrtime2 = gethrtime_x86();
        printf("Hrtime = %f seconds, val_counter = %lu\n", hrtime2 - hrtime1, val_counter);
    
    }
    return NULL;
}

void *work_thread3(void *id){
    int task_id = *((int *) id);
    int i;
    double hrtime1, hrtime2;
    barrier(task_id);
    pthread_t  thread_id;
    cpu_set_t cpuset;
    int set_aff;

    thread_id = pthread_self(); // get the ID of calling thread
    CPU_ZERO(&cpuset);
    CPU_SET(task_id, &cpuset); // set specified CPU in a set

    set_aff = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset); // set thread I to core I
    assert(set_aff == 0);

    if (task_id == 0) {
        val_counter = 0;
        flag = 0;
        hrtime1 = gethrtime_x86();
        printf("Now start all the tasks with TAS_LOCK, val_counter = %lu\n", val_counter);
    }
    barrier(task_id);
    for(i = 0; i < num_count; i++){
        while (tas(&flag) == 1);  //acquire tas lock, either get the lock or spin
        val_counter++;
        flag = 0;  //release tas lock
    }
    barrier(task_id);
    if (task_id == 0) {
        hrtime2 = gethrtime_x86();
        printf("Hrtime = %f seconds, val_counter = %lu\n", hrtime2 - hrtime1, val_counter);
    
    }
    return NULL;
}

void tatas_lock(volatile unsigned long* flag){
    while (1){
        while(*flag != 0);
        if(tas(flag) == 0)
            return;
    }
}

void tatas_unlock(volatile unsigned long* flag){
    *flag = 0;  //reset the flag
}

void *work_thread4(void *id){
    int task_id = *((int *) id);
    int i;
    double hrtime1, hrtime2;
    barrier(task_id);
    pthread_t  thread_id;
    cpu_set_t cpuset;
    int set_aff;

    thread_id = pthread_self(); // get the ID of calling thread
    CPU_ZERO(&cpuset);
    CPU_SET(task_id, &cpuset); // set specified CPU in a set

    set_aff = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset); // set thread I to core I
    assert(set_aff == 0);

    if (task_id == 0) {
        val_counter = 0;
        flag = 0;
        hrtime1 = gethrtime_x86();
        printf("Now start all the tasks with TATAS_LOCK, val_counter = %lu\n", val_counter);
    }
    barrier(task_id);
    for(i = 0; i < num_count; i++){
        tatas_lock(&flag);  //acquire tatas lock, either get the lock or spin
        val_counter++;
        tatas_unlock(&flag);  //release tas lock
    }
    barrier(task_id);
    if (task_id == 0) {
        hrtime2 = gethrtime_x86();
        printf("Hrtime = %f seconds, val_counter = %lu\n", hrtime2 - hrtime1, val_counter);
    
    }
    return NULL;
}

void *work_thread5(void *id){
    int task_id = *((int *) id);
    int i;
    double hrtime1, hrtime2;
    barrier(task_id);
    pthread_t  thread_id;
    cpu_set_t cpuset;
    int set_aff;

    thread_id = pthread_self(); // get the ID of calling thread
    CPU_ZERO(&cpuset);
    CPU_SET(task_id, &cpuset); // set specified CPU in a set

    set_aff = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset); // set thread I to core I
    assert(set_aff == 0);

    if (task_id == 0) {
        val_counter = 0;
        flag = 0;
        hrtime1 = gethrtime_x86();
        printf("Now start all the tasks with TATAS_WITH_BACKOFF_LOCK, val_counter = %lu\n", val_counter);
    }
    barrier(task_id);
    for(i = 0; i < num_count; i++){
        tatas_acquire(&flag);  //acquire tatas lock, either get the lock or spin
        val_counter++;
        tatas_release(&flag);  //release tas lock
    }
    barrier(task_id);
    if (task_id == 0) {
        hrtime2 = gethrtime_x86();
        printf("Hrtime = %f seconds, val_counter = %lu\n", hrtime2 - hrtime1, val_counter);
    
    }
    return NULL;
}

void ticket_init(ticket_lock_t* tick){
    tick->next_ticket = 0;
    tick->now_serving = 0;
}

void *work_thread6(void *id){
    int task_id = *((int *) id);
    int i;
    double hrtime1, hrtime2;
    barrier(task_id);
    pthread_t  thread_id;
    cpu_set_t cpuset;
    int set_aff;

    thread_id = pthread_self(); // get the ID of calling thread
    CPU_ZERO(&cpuset);
    CPU_SET(task_id, &cpuset); // set specified CPU in a set

    set_aff = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset); // set thread I to core I
    assert(set_aff == 0);

    if (task_id == 0) {
        val_counter = 0;
        ticket_init(&tick);
        hrtime1 = gethrtime_x86();
        printf("Now start all the tasks with TICKET_LOCK, val_counter = %lu\n", val_counter);
    }
    barrier(task_id);
    for(i = 0; i < num_count; i++){
        ticket_acquire(&tick);  //acquire tatas lock, either get the lock or spin
        val_counter++;
        ticket_release(&tick);  //release tas lock
    }
    barrier(task_id);
    if (task_id == 0) {
        hrtime2 = gethrtime_x86();
        printf("Hrtime = %f seconds, val_counter = %lu\n", hrtime2 - hrtime1, val_counter);
    
    }
    return NULL;
}

void mcs_init(mcs_qnode_t** L){
    *L = 0;
}

void *work_thread7(void *id){
    int task_id = *((int *) id);
    int i;
    double hrtime1, hrtime2;
    mcs_qnode_t I; //mcs lock node for every thread
    barrier(task_id);
    pthread_t  thread_id;
    cpu_set_t cpuset;
    int set_aff;

    thread_id = pthread_self(); // get the ID of calling thread
    CPU_ZERO(&cpuset);
    CPU_SET(task_id, &cpuset); // set specified CPU in a set

    set_aff = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset); // set thread I to core I
    assert(set_aff == 0);


    if (task_id == 0) {
        val_counter = 0;
        mcs_init(&L);
        hrtime1 = gethrtime_x86();
        printf("Now start all the tasks with MCS_LOCK, val_counter = %lu\n", val_counter);
    }
    barrier(task_id);
    for (i = 0; i < num_count; i++) {
        mcs_acquire(&L, &I);  //acquire tatas lock, either get the lock or spin
        val_counter++;
        mcs_release(&L, &I);  //release tas lock
    }
    barrier(task_id);
    if (task_id == 0) {
        hrtime2 = gethrtime_x86();
        printf("Hrtime = %f seconds, val_counter = %lu\n", hrtime2 - hrtime1, val_counter);
    
    }
    return NULL;
}

void *work_thread8(void *id){
    int task_id = *((int *) id);
    int i;
    double hrtime1, hrtime2;
    barrier(task_id);
    pthread_t  thread_id;
    cpu_set_t cpuset;
    int set_aff;

    thread_id = pthread_self(); // get the ID of calling thread
    CPU_ZERO(&cpuset);
    CPU_SET(task_id, &cpuset); // set specified CPU in a set

    set_aff = pthread_setaffinity_np(thread_id, sizeof(cpu_set_t), &cpuset); // set thread I to core I
    assert(set_aff == 0);

    if (task_id == 0) {
        val_counter = 0;
        hrtime1 = gethrtime_x86();
        printf("Now start all the tasks with FAI, val_counter = %lu\n", val_counter);
    }
    barrier(task_id);
    for (i = 0; i < num_count; i++) {
        fai(&val_counter);
    }
    barrier(task_id);
    if (task_id == 0) {
        hrtime2 = gethrtime_x86();
        printf("Hrtime = %f seconds, val_counter = %lu\n", hrtime2 - hrtime1, val_counter);
    
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
                    if ((m > 0) && (m < 9)){
                        lock_mode = m;
                    } else {
                        fprintf(stderr,"Please confirm your selection of lock mode, default will be no synchronization:\n1 : No sync; \n2 : With pthread lock;\n3 : With TAS lock;\n4 : With TATAS lock;\n5 : With backoff included in TATAS lock;\n6 : With Ticked locks;\n7 : With MCS locks;\n8 : With FAI;\n");
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
            case 3: pthread_create(&tid[i], NULL, work_thread3, &id[i]); break;
            case 4: pthread_create(&tid[i], NULL, work_thread4, &id[i]); break;
            case 5: pthread_create(&tid[i], NULL, work_thread5, &id[i]); break;
            case 6: pthread_create(&tid[i], NULL, work_thread6, &id[i]); break;
            case 7: pthread_create(&tid[i], NULL, work_thread7, &id[i]); break;
            case 8: pthread_create(&tid[i], NULL, work_thread8, &id[i]); break;
        	default: errexit ("Lock mode error\n");
        }

    }

    /* ============ Thread Join ============ */

    for(i = 0; i < num_th; i++){
        pthread_join(tid[i], NULL);
    }


    return 0;



}