/* 
 * Tsivgiouras Kon/nos, AEM: 2378 (ktsivgiouras@uth.gr)
 * Koffas Georgios, AEM: 2389 (gkoffas@uth.gr)
 * 
 * The following is a programm that tests whether a set of integers is a prime or not through concurrently assigning
 * "jobs" to worker threads from the main thread. After a worker is done with the primality test, main sends another
 * integer for evaluation, and so forth. This process stops once all workers are done, and no more jobs are available.
 * 
 * Program is run like this:
 * 	./prime <n> <(input_file) >(output_file)
 * 
 * Saturday, 13/10/2018
 * University of Thessaly, Department of Electrical & Computer Engineering
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

/* Defines the maximum string of integers that user will input. */
#define MAX_PRIMES 500

/* Defines the maximum number of worker threads the program will have. */
#define MAX_WORKERS 1000

#ifdef DEBUG
#endif

struct worker {
	
	unsigned long int possible_prime;
	int ready; /* 1 = worker ready , 0 = worker unavailable. */   
	int start_working;	/* 0 if worker has not been told to start working, 1 if yes. */
	int termination; /* 1 = terminate signal , 2 = tell main i terminated , 0 nothing. */
};

typedef struct worker worker_t;

/* The array of workers where main thread assigns jobs. */
worker_t table_of_workers[MAX_WORKERS];


/* A simple primality test algorithm. */
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

/* The worker thread function. */
void *worker_thread(void *arg){
	
	int *temp;
	int worker_id;
	int flag = 0;
	int is_prime;
	
	temp = (int *)arg;
	worker_id = *temp;
	
	/* Used for debuggind purposes. */
	//fprintf(stderr, "worker: %d\n", worker_id);
	while(1) {
		
		/* Worker is ready to start. */
		table_of_workers[worker_id].ready = 1;
		
		/* Busy waiting. */
		while(table_of_workers[worker_id].start_working == 0) {
			if(table_of_workers[worker_id].termination == 1) {	/* If signaled to terminate. */
				flag = 1;
				break;
			}
		}
		
		if(flag == 1) {
			break;
		}
		
		is_prime = isprime(table_of_workers[worker_id].possible_prime);
		
		if(is_prime == 1) {
			printf("%lu is a prime number!\n",table_of_workers[worker_id].possible_prime);
		}
		else{
			printf("%lu is not a prime number!\n",table_of_workers[worker_id].possible_prime);
		}
		
		/* Ready to start working again. */
		table_of_workers[worker_id].start_working = 0;
	}
	
	/* Signaling main that I have terminated. */
	table_of_workers[worker_id].termination = 2;
	return NULL;
	
}


int main (int argc,char* argv[]) {
	
	int number_of_workers;
	int i = 0;	/* Used to traverse the table_of_workers array. */
	int j = 0;	/* Used tp traverse the table_of_numbers array. */
	int counter = 0;
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
	
	table_of_numbers[i] = temp;
	
	/* Initialization of the workers. */
	for(i=0; i < number_of_workers; i++) {
		
		table_of_workers[i].ready = 0;
		table_of_workers[i].start_working = 0;
		table_of_workers[i].termination = 0;
		turn[i] = i;
	}
	
	for(i=0; i < number_of_workers; i++) {
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
				table_of_workers[i].ready = 0;
				table_of_workers[i].start_working = 1;
				j++;
			}
		}
	}
	
	/* Used for debugging purposes. */
	//fprintf(stderr, "end 1st while\n");
	
	
	/* Waiting for all workers to finish their job. */
	while(1) {
		for(i=0; i < number_of_workers; i++) {
			if(table_of_workers[i].ready == 1) {
				counter++;
			}
		}

		/* Used for debugging purposes. */
		//fprintf(stderr, "counter: %d\n", counter);
		//fprintf(stderr, "number_of_workers: %d\n", number_of_workers);
		//sleep(1);
		
		if(counter == number_of_workers){
			for(i=0; i < number_of_workers; i++) {
				table_of_workers[i].termination = 1;
			}
			break;
		}
		else{
			counter =0;
		}
	}
	
	counter=0;
	
	/* Waiting for all workers to terminate. */
	while(1) {
		for(i=0; i < number_of_workers; i++) {
			if(table_of_workers[i].termination == 2) {
				counter++;
			}
		}
		if(counter == number_of_workers){
				break;
		}
		else{
			counter =0;
		}
	}
	
	return(0);
}
