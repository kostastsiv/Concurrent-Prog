/* 
 * Tsivgiouras Kon/nos, AEM: 2378 (ktsivgiouras@uth.gr)
 * Koffas Georgios, AEM: 2389 (gkoffas@uth.gr)
 * 
 * Simple primality test algorithm using multi-user-level-threading, utilising
 * our user-level thread API.
 * 
 * Friday 21/12/2018
 * University of Thessaly, Department of Electrical & Computer Engineering
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include "mythreads_t.h"

#define MAX_WORKERS 100	/* Maximum number of user-level threads */
#define MAX_PRIMES 200	/* Maximum number of primes for testing */

struct worker {
	
	unsigned long int possible_prime;
	int ready; /* 1 = worker ready , 0 = worker unavailable. */   
	sem_t  worker_id_sem;
	int termination;
};

typedef struct worker worker_t;

thr_t *head;
thr_t *scheduler_p;
struct sigaction act_def;
struct sigaction act_ign;
int id = 1;
thr_t thread_array[MAX_WORKERS];	/* Table of threads to create. */
thr_t main_co;
worker_t table_of_workers[MAX_WORKERS];	/* Table of workers. */

/* Primality test function. */
int isprime(unsigned long int n) {
	
	int i;
	
	if (n <=1) {
		
		return 0;
	}
	
	if(n == 2) {
		
		return 1;
	}
	
	if( n%2 == 0) {
		return 0;
	}
	
	for(i=3; i <= floor(sqrt(n)); i+=2) {
		
		if (n%i == 0) {
			
			return 0;
		}
	}
	
	return 1;
	
}

/* Worker function. */
void worker(void*arg) {
	int worker_id;
	int is_prime;
	
	worker_id = *((int *)arg);
	
	while(1) {
		table_of_workers[worker_id].ready = 1;	/* Worker is ready. */
		mythreads_sem_down(&(table_of_workers[worker_id].worker_id_sem));	/* Wait for job by main. */
		
		if(table_of_workers[worker_id].termination == 1) {	/* If signaled to terminate. */
			break;
		}
		
		is_prime = isprime(table_of_workers[worker_id].possible_prime);
		
		if(is_prime == 1) {
			printf("thread id %lu : %lu is a prime number!\n",scheduler_p->id,table_of_workers[worker_id].possible_prime);
		}
		else{
			printf("thread id %lu : %lu is not a prime number!\n",scheduler_p->id,table_of_workers[worker_id].possible_prime);
		}
		
	}
	
	mythreads_destroy(scheduler_p);
}

int main (int argc,char*argv[]) {
	
	int number_of_workers;
	int how_many_numbers;
	int i=0;
	int j=0;
	ucontext_t main_c;
	long unsigned int table_of_numbers[MAX_PRIMES];
	long unsigned int temp;
	int turn [MAX_WORKERS];
	sem_t termination_sems[MAX_PRIMES];
	
	number_of_workers = atoi(argv[1]);
	
	scanf(" %lu",&temp);
	
	while(temp != 0) {
		table_of_numbers[i] = temp;
		scanf(" %lu",&temp);
		i++;
	}
	
	how_many_numbers = i;
	
	table_of_numbers[i] = temp;
	
	for(i=0; i < number_of_workers; i++) {
		table_of_workers[i].ready = 0;
		turn[i] = i;
	}
	
	mythreads_init(main_c);
	
	for(i = 0; i < how_many_numbers; i++) {
        mythreads_sem_init(&(termination_sems[i]),0);
    }
    
	for(i=0; i < number_of_workers; i++) {
		
        mythreads_sem_init(&(table_of_workers[i].worker_id_sem),0);
		mythreads_create(&(thread_array[i]),(void*)(void*)worker,(void *)&turn[i]);
	}
	
	while(table_of_numbers[j] > 0){
		for(i=0; i < number_of_workers; i++) {
			if(table_of_workers[i].ready == 1) {
				table_of_workers[i].possible_prime = table_of_numbers[j];
				table_of_workers[i].ready = 0;
				mythreads_sem_up(&(table_of_workers[i].worker_id_sem));
				j++;
                break;
			}
		}
	}
	mythreads_yield();	/* Yields are used for round-robin faithfulness. */
	
	for(i=0; i < number_of_workers; i++) {
		table_of_workers[i].termination = 1;
		mythreads_sem_up(&(table_of_workers[i].worker_id_sem));
	}
	
	mythreads_yield();
	
	for(i=0; i < number_of_workers; i++) {
		mythreads_join(&thread_array[i]);
	}
	
	mythreads_destroy(head);
	
	return(0);
}
