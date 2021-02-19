/* 
 * Tsivgiouras Kon/nos, AEM: 2378 (ktsivgiouras@uth.gr)
 * Koffas Georgios, AEM: 2389 (gkoffas@uth.gr)
 * 
 * This program simulates a train ride. It takes the number of total customers
 * and the number of customers per ride as input arguments.
 * 
 * Friday, 30/11/2018
 * University of Thessaly, Department of Electrical & Computer Engineering
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_CUSTOMERS 4000

pthread_mutex_t mtx;
pthread_cond_t train_sleep;
pthread_cond_t customer_waits;
volatile int customers = 0;

struct thread_arg {
	int sleep_time;
	int customers_per_ride;
	int number_of_customers;
};

typedef struct thread_arg thread_arg_t;


void * train_thread(void *arg) {
	
	int check;
	int div;
	int counter = 0;
	int i;
	thread_arg_t *input = (thread_arg_t*)arg;
	
	/* Max number of rides possible. */
	div = (input->number_of_customers)/(input->customers_per_ride);
	
	while(1) {
		
		check = pthread_mutex_lock(&mtx);
		
		if(check != 0) {
			fprintf(stderr,"Mutex_lock failed in train_thread, errno: %d,line: %d\n",errno,__LINE__);
			exit(errno);
		}
		
		counter++;
		
		if(counter > div) {
			printf("Train finished for the day.\n"); 
			break;
		}
		
		customers -= input->customers_per_ride;
		
		if(customers  < 0) {
			printf("Train waits for the last customer.\n");
			check = pthread_cond_wait(&train_sleep,&mtx);
			
			if(check != 0 ) {
				fprintf(stderr,"Cond_wait failed in train_thread, errno: %d,line: %d\n",errno,__LINE__);
				exit(errno);
			}
		}
		
		printf("Train starts the ride!\n");
		sleep(input->sleep_time);
		
		/* Wake up all waiting customers: ride is ready to start again. */
		for(i=0; i < input->customers_per_ride ; i++) {
			
			pthread_cond_signal(&customer_waits);
			
		}
		
		check = pthread_mutex_unlock(&mtx);
		
		if(check != 0 ) {
			fprintf(stderr,"Mutex_unlock failed in train_thread, errno: %d,line: %d\n",errno,__LINE__);
			exit(errno);
		}
	}
	
	check = pthread_mutex_unlock(&mtx);
		
	if(check != 0 ) {
		fprintf(stderr,"Mutex_unlock failed in train_thread, errno: %d,line: %d\n",errno,__LINE__);
		exit(errno);
	}
	
	return NULL;
}

void * customer_thread(void*arg) {
	
	thread_arg_t *input = (thread_arg_t*)arg;
	int check;
	
	check = pthread_mutex_lock(&mtx);
	
	if(check != 0 ) {
		fprintf(stderr,"Mutex_lock failed in customer_thread,errno: %d,in line: %d\n",errno,__LINE__);
		exit(errno);
	}
	
	customers ++;
	
	printf("Customer : %lu(thread) arrived.\n",pthread_self());
	
	if(customers == 0) {	/* The ride is ready to start. */
		
		printf("Last customer arived : %lu(thread)\n",pthread_self());
		sleep(input->sleep_time);
		check = pthread_cond_signal(&train_sleep);
		if(check != 0) {
			fprintf(stderr,"Cond_signal failed in customer_thread,errno: %d,in line: %d\n",errno,__LINE__);
			exit(errno);
		}
	}
	else {	/* The customers wait for the ride to be empty again. */
		printf("Customer : %lu(thread) waits for train.\n",pthread_self());
		sleep(input->sleep_time);
		pthread_cond_wait(&customer_waits,&mtx);
	}
	check = pthread_mutex_unlock(&mtx);

	if(check != 0 ) {
		fprintf(stderr,"Mutex_unlock failed in customer_thread,errno: %d,in line: %d\n",errno,__LINE__);
		exit(errno);
	}
	
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
	arguments.number_of_customers = atoi(argv[1]);
	
	check = pthread_create(&train,NULL,train_thread,&arguments);
	
	if(check!=0) {
		fprintf(stderr,"Error in pthread_create line:%d\n",__LINE__);
		return(1);
	}
	
	for(i = 0; i < arguments.number_of_customers; i++) {
		
		check = pthread_create(&customers[i],NULL,customer_thread,&arguments);
		
		if(check!=0) {
			fprintf(stderr,"Error in pthread_create line:%d\n",__LINE__);
			return(1);
		}
		sleep(1);
	}
	
	for(i=0; i < arguments.number_of_customers; i++) {
		
		pthread_join(customers[i],NULL);
	}
	
	pthread_join(train,NULL);
	
	
	return(0);
}
