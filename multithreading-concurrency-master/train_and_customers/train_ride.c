#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "bsem.h"

#define MAX_CUSTOMERS 40

volatile int mtx;
volatile int train_wait;
volatile int termination_sig;
volatile int customers = 0;


struct thread_arg {
	int sleep_time;
	int number_of_max_customers;
	int customers_per_ride;
};

typedef struct thread_arg thread_arg_t;

void * train_thread(void *arg) {
	
	thread_arg_t *input = (thread_arg_t*)arg;
	int counter = 0;
	int div;
	
	div = input->number_of_max_customers / input->customers_per_ride;
	
	while(1) {
		
		mysem_down(mtx);
		counter++;
		if (counter > div) {
			break;
		}
		customers -= input->customers_per_ride;
		
		if(customers < 0) {
			mysem_up(mtx);
			printf("train waits for the last customer\n");
			mysem_down(train_wait);
			mysem_down(mtx);
		}
		
		printf("Train starts the ride\n");
		//sleep(input->sleep_time);
		
		mysem_up(mtx);
	}
	
	mysem_up(termination_sig);
	return NULL;
}

void * customer_thread(void*arg) {
	
	thread_arg_t *input = (thread_arg_t*)arg;
	
	mysem_down(mtx);
	
	customers++;
	
	if(customers == 0) {
		
		printf("Last customer arived\n");
		//sleep(input->sleep_time);
		mysem_up(train_wait);
	}
	
	mysem_up(mtx);
	return NULL;
}

int main(int argc,char* argv[]) {

	int i ;
	int check;
	thread_arg_t arguments;
	pthread_t train;
	pthread_t customers[MAX_CUSTOMERS];
	
	if(argc != 4) {
		printf("Wrong number of arguments\n");
		return(1);
	}
	
	arguments.sleep_time = atoi(argv[3]);
	arguments.customers_per_ride = atoi(argv[2]);
	arguments.number_of_max_customers = atoi(argv[1]);
	
	mtx = mysem_create(1);
	train_wait = mysem_create(0);
	termination_sig = mysem_create(0);
	
	check = pthread_create(&train,NULL,train_thread,&arguments);
	
	if(check!=0) {
		fprintf(stderr,"Error in pthread_create line:%d\n",__LINE__);
		return(1);
	}
	
	for(i = 0; i < arguments.number_of_max_customers; i++) {
		
		check = pthread_create(&customers[i],NULL,customer_thread,&arguments);
		
		if(check!=0) {
			fprintf(stderr,"Error in pthread_create line:%d\n",__LINE__);
			return(1);
		}
// 		sleep(1);
	}
	
	mysem_down(termination_sig);
	
	mysem_destroy(mtx);
	mysem_destroy(train_wait);
	mysem_destroy(termination_sig);
	
	return(0);
}
		
