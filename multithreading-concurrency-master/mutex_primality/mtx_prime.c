/* 
 * Koffas Georgios, AEM:2389 (gkoffas@uth.gr)
 * Tsivgiouras Kon/nos, AEM:2378 (ktsivgiouras@uth.gr)
 *
 * This program is a simple primality test, utilising monitor
 * logic via multithreading.
 *
 * Wednesday, 28/11/2018
 * University of Thessaly, Department of Electrical & Computer Engineering
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

/* Maximum number of primes fed as input. */
#define MAX_PRIMES 500
/* Maximum number of worker threads the program can handle. */
#define MAX_WORKERS 1000

struct worker {
	unsigned long int possible_prime;
	int ready; /* 1 = worker ready , 0 = worker unavailable. */   
	pthread_cond_t worker_cond;
	};

typedef struct worker worker_t;

worker_t table_of_workers[MAX_WORKERS];	/* An array storing all workers. */
pthread_mutex_t mtx;
pthread_cond_t termination_cond;	/* Used by main to know when all
					   workers have finished. */
volatile int counter = 0;		/* Indicates number of primes that
					   has been run through the primality
					   test. */
volatile int how_many_numbers;
volatile int number_of_workers;
volatile int termination = 0;		/* Assigned to 1 when all jobs
					   have been assigned. */

/* The primality test function. */
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

/* Monitor routine for wait of worker. */
int worker_wait(int i){
	
	int check;
	
	check = pthread_mutex_lock(&mtx);
	
	if(check != 0) {
		
		fprintf(stderr,"Mutex_lock failed in worker_wait,errno: %d, in line: %d\n",check,__LINE__); 
		
		return(-1);
	}
	
	table_of_workers[i].ready = 1;
	
	check = pthread_cond_wait(&(table_of_workers[i].worker_cond),&mtx);
	
	if(termination == 1) { 
		
		check = pthread_mutex_unlock(&mtx);
	
		if(check != 0) {
		
			fprintf(stderr,"Mutex_unlock failed in worker_wait,errno: %d, in line: %d\n",check,__LINE__); 
			return(-1);
		}
		
		return(1);
		
	}
	
	if(check != 0) {
		
		fprintf(stderr,"Cond_wait failed in worker_wait, worker who called: %d,errno: %d, in line: %d\n",i,check,__LINE__); 
		
		return(-1);
	}
	
	check = pthread_mutex_unlock(&mtx);
	
	if(check != 0) {
		
		fprintf(stderr,"Mutex_unlock failed in worker_wait,errno: %d, in line: %d\n",check,__LINE__); 
		return(-1);
	}
	return(0);
}

/* Monitor routine for main assigning prime numbers to workers. */
int main_assign_prime(int i) {
	
	int check;
	
	check = pthread_mutex_lock(&mtx);
	
	if(check != 0) {
		
		fprintf(stderr,"Mutex_lock failed in main_assign, errno: %d, in line: %d\n",check,__LINE__); 
		
		return(check);
	}
	
	table_of_workers[i].ready = 0;
	
	check = pthread_cond_signal(&(table_of_workers[i].worker_cond));
	
	if(check != 0) {
		
		fprintf(stderr,"Cond_signal failed in main_assign, errno: %d, in line: %d\n",check,__LINE__); 
		
		return(check);
	}
	
	check = pthread_mutex_unlock(&mtx);
	
	if(check != 0) {
		
		fprintf(stderr,"Mutex_unlock failed in main_assign, errno: %d, in line: %d\n",check,__LINE__); 
		
		return(check);
	}
	
	return(0);
}

/* Monitor routine for worker finishing job. */
int worker_finish(){
	
	int check;
	
	check = pthread_mutex_lock(&mtx);
	
	if(check != 0) {
		
		fprintf(stderr,"Mutex_lock failed in worker_finish,errno: %d, in line: %d\n",check,__LINE__); 
		
		return(check);
	}
	
	counter ++;
	
	if(counter == how_many_numbers) {
		
		check = pthread_cond_signal(&termination_cond);	
		
		if(check!=0) {
			
			fprintf(stderr,"Cond_signal failed in worker_finish,errno: %d, in line: %d\n",check,__LINE__); 
			return(check);
		}
	}
	
	check = pthread_mutex_unlock(&mtx);
	
	if(check != 0) {
		
		fprintf(stderr,"Mutex_unlock failed in worker_finish,errno: %d, in line: %d\n",check,__LINE__); 
		return(check);
	}
	
	return(0);
	
}

/* Monitor routine for main finishing assigning numbers. */
int main_finish() {
	
	int check;
	int i;
	
	check = pthread_mutex_lock(&mtx);
	
	if(check != 0) {
		
		fprintf(stderr,"Mutex_lock failed in main_finish,errno: %d, in line: %d\n",check,__LINE__); 
		
		return(check);
	}
	
	if(counter < how_many_numbers) {
		
		check = pthread_cond_wait(&termination_cond,&mtx);
		
		if(check != 0) {
			
			fprintf(stderr,"Cond_wait failed in main_finish,errno: %d, in line: %d\n",check,__LINE__); 
			
			return(check);
		}
	}
		
	termination = 1;
	
	for(i=0; i < number_of_workers; i++) {
		
		check = pthread_cond_signal(&(table_of_workers[i].worker_cond));
		
		if(check != 0) {
			
			fprintf(stderr,"Cond_signal failed in main_finish,errno: %d, in line: %d\n",check,__LINE__); 
			
			return(check);
		}
	}
	
	check = pthread_mutex_unlock(&mtx);
	
	if(check != 0) {
		
		fprintf(stderr,"Mutex_unlock failed in main_finish,errno: %d, in line: %d\n",check,__LINE__); 
		
		return(check);
	}
	
	return(0);
}
	
/* The worker thread. */
void *worker_thread(void *arg){
	
	int worker_id;
	int is_prime;
	int check;
	
	worker_id = *((int *)arg);
	
	while(1) {
		
		check = worker_wait(worker_id);
		
		if(check == -1) {
			exit(errno);
		}
		
		if(check == 1) {
			printf("thread: %lu is exiting\n",pthread_self());
			break;
		}
		
		/* CS */
		is_prime = isprime(table_of_workers[worker_id].possible_prime);
		
		if(is_prime == 1) {
			printf("Thread %lu says: %lu is a prime number!\n",pthread_self(), table_of_workers[worker_id].possible_prime);
		}
		else{
			printf("Thread %lu says: %lu is not a prime number!\n",pthread_self(), table_of_workers[worker_id].possible_prime);
		}
		
		check = worker_finish();
		
		if(check != 0) {
			exit(errno);
		}
	}
	return NULL;
	
}

int main (int argc,char* argv[]) {
	
	int i = 0;	/* Used to traverse the table_of_workers array. */
	int j = 0;	/* Used tp traverse the table_of_numbers array. */
	int check;
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
	
	
	for(i=0; i < number_of_workers; i++) {
        
		check = pthread_create(&worker_id_table[i],NULL,worker_thread,(void *)&turn[i]);
		
		if(check != 0) {
			fprintf(stderr,"Problem in pthread_create, worker_id: %d,in line: %d\n",turn[i],__LINE__);
			_exit(EXIT_FAILURE);
		}
	}
	
	/* Waiting for all numbers to be checked for primality. */
	
	while(table_of_numbers[j] > 0){
		for(i=0; i < number_of_workers; i++) {
			if(table_of_workers[i].ready == 1) {
				table_of_workers[i].possible_prime = table_of_numbers[j];
				check = main_assign_prime(i);
				
				if(check != 0) {
					exit(errno);
				}
				
				j++;
             			break;
			}
		}
	}
	
	check = main_finish();
	
	if(check != 0) {
		exit(errno);
	}
	
	for(i = 0; i < number_of_workers; i++) {
		
		pthread_join(worker_id_table[i],NULL);
	}
	
	return(0);
}

