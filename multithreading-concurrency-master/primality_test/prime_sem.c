#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "bsem.h"


#define MAX_PRIMES 500
#define MAX_WORKERS 1000

struct worker {
	unsigned long int possible_prime;
	int ready; /* 1 = worker ready , 0 = worker unavailable. */   
	int worker_id_sem;
    int termination_sem;
};

typedef struct worker worker_t;

worker_t table_of_workers[MAX_WORKERS];

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

void *worker_thread(void *arg){
	
	int worker_id;
	int is_prime;
	
	worker_id = *((int *)arg);
	
	/* Used for debuggind purposes. */
	//fprintf(stderr, "worker: %d\n", worker_id);
	while(1) {
		
		/* Worker is ready to start. */
		table_of_workers[worker_id].ready = 1;
		
		/* waiting. */
		mysem_down(table_of_workers[worker_id].worker_id_sem);
		
		is_prime = isprime(table_of_workers[worker_id].possible_prime);
		
		if(is_prime == 1) {
			printf("%lu is a prime number!\n",table_of_workers[worker_id].possible_prime);
		}
		else{
			printf("%lu is not a prime number!\n",table_of_workers[worker_id].possible_prime);
		}
		
		mysem_up(table_of_workers[worker_id].termination_sem);
	}
	
	/* Signaling main that I have terminated. */
	return NULL;
	
}

int main (int argc,char* argv[]) {
	
	int number_of_workers;
	int i = 0;	/* Used to traverse the table_of_workers array. */
	int j = 0;	/* Used tp traverse the table_of_numbers array. */
	int how_many_numbers;
	int temp_id;
	int check;
	int termination_sems[MAX_PRIMES];
	int turn [MAX_WORKERS];	/* An array storing the "ID" of each worker. */
	long unsigned int temp;
	long unsigned int table_of_numbers[MAX_PRIMES];
	pthread_t worker_id_table[MAX_WORKERS];
	
	
	if (argc != 2) {
		fprintf(stderr,"Wrong number of arguments!\n");
		return (1);
	}
	
	number_of_workers = atoi(argv[1]);
	
	scanf(" %lu",&temp);
	
	while(temp != 0) {
		table_of_numbers[i] = temp;
		scanf(" %lu",&temp);
		i++;
	}
	
	how_many_numbers = i;
	
	table_of_numbers[i] = temp;
	
	/* Initialization of the workers. */
	for(i=0; i < number_of_workers; i++) {
		
        table_of_workers[i].ready = 0;
		turn[i] = i;
        
	}
	
	for(i = 0; i < how_many_numbers; i++) {
        
        termination_sems[i] = mysem_create(0);
        
    }
	
	for(i=0; i < number_of_workers; i++) {
        
        temp_id = mysem_create(0);
        table_of_workers[i].worker_id_sem = temp_id;
		check = pthread_create(&worker_id_table[i],NULL,worker_thread,(void *)&turn[i]);
		
		if(check != 0) {
			fprintf(stderr,"problem in pthread_create\n");
			_exit(EXIT_FAILURE);
		}
	}
	
	/* Waiting for all numbers to be checked for primality. */
	while(table_of_numbers[j] > 0){
		for(i=0; i < number_of_workers; i++) {
			if(table_of_workers[i].ready == 1) {
				table_of_workers[i].possible_prime = table_of_numbers[j];
                table_of_workers[i].termination_sem = termination_sems[j];
				table_of_workers[i].ready = 0;
				mysem_up(table_of_workers[i].worker_id_sem);
				j++;
                break;
			}
		}
	}
	
	for(i = 0; i < how_many_numbers; i++) {
        mysem_down(termination_sems[i]);
    }
	
	/* Destorying all semaphores */
	for (i = 0; i < number_of_workers; i++) {
		mysem_destroy(table_of_workers[i].worker_id_sem);
	}
	
	return(0);
}



