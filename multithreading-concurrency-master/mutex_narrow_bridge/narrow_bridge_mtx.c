/*
 * Koffas Georgios, AEM: 2389 (gkoffas@uth.gr)
 * Tsivgiouras Kon/nos, AEM: 2378 (ktsivgiouras@uth.gr)
 *
 * This program simulates a narrow bridge problem, utilising monitor logic via
 * multithreading.
 *
 * Wednesday, 28/11/2018
 * University of Thessaly, Department of Electrical & Computer Engineering
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_CARS 500
#define MAX_LIMIT 50

struct car {
	int sleep_time;
	int number_of_max_cars;
};

typedef struct car car_T;

volatile int how_many_red_cars = 0;
volatile int how_many_blue_cars = 0;
volatile int counter_blue = 0;
volatile int counter_red = 0;
volatile int red_queue = 0;
volatile int blue_queue = 0;
pthread_mutex_t mtx;
pthread_cond_t blue_traffic;
pthread_cond_t red_traffic;

/* Custom functions used for error handling. */
int my_pthread_mutex_lock(pthread_mutex_t * mtx){
	
	int check;
	
	check = pthread_mutex_lock(mtx);
	
	if(check != 0) {
		
		fprintf(stderr,"Mutex_lock failure,errno: %d\n",errno);
		
		exit(-1);
	}
	
	return(0);
}

int my_pthread_mutex_unlock(pthread_mutex_t *mtx) {
	
	int check;
		
		check = pthread_mutex_unlock(mtx);
		
		if(check != 0) {
			
			fprintf(stderr,"Mutex_unlock failure,errno: %d\n",errno);
			
			exit(-1);
		}
		
		return(0);
}

int my_pthread_cond_wait(pthread_cond_t *cond ,pthread_mutex_t* mtx) {

	int check;
		
		check = pthread_cond_wait(cond,mtx);
		
		if(check != 0) {
			
			fprintf(stderr,"Cond_wait failure,errno: %d\n",errno);
			
			exit(-1);
		}
		
		return(0);
}

int my_pthread_cond_signal(pthread_cond_t *cond) {

	int check;
		
		check = pthread_cond_signal(cond);
		
		if(check != 0) {
			
			fprintf(stderr,"Cond_signal failure,errno: %d\n",errno);
			
			exit(-1);
		}
		
		return(0);
}

/* Monitor routine for when a red car tries to enter the bridge. */
void red_car_enters_bridge(car_T* input) {
	
	my_pthread_mutex_lock(&mtx);
	
	how_many_red_cars ++;
	
	if(how_many_red_cars > input->number_of_max_cars) {
		
		red_queue ++;
		
		my_pthread_cond_wait(&red_traffic,&mtx); 
		
	}
	
	if(counter_blue > 0) {
		
		red_queue++;
		
		my_pthread_cond_wait(&red_traffic,&mtx);
	}
	
	counter_red++;
	
	my_pthread_mutex_unlock(&mtx);
	
	return;
}

/* Monitor routine for when a red car exits the bridge. */
void red_car_exits(car_T *input) {
	
	int i;
	int temp;
	
	my_pthread_mutex_lock(&mtx);
	
	counter_red --;
	
	if(counter_red == 0) {
		
		if(how_many_red_cars > 0 && how_many_blue_cars ==0) {
			
			if(red_queue > input->number_of_max_cars) {
				
				red_queue -= input->number_of_max_cars;
				
				for(i = 0; i < input->number_of_max_cars; i++) {
					
					my_pthread_cond_signal(&red_traffic);
				}
			}
			else{
				
				temp = red_queue;
				red_queue = 0;
				
				for(i=0; i < temp; i++) {
					
					my_pthread_cond_signal(&red_traffic);
				}
			}
			how_many_red_cars --;
			my_pthread_mutex_unlock(&mtx);
			return;
		}
		
		else if(how_many_blue_cars > 0) {
			
			if(blue_queue > input->number_of_max_cars) {
				
				blue_queue -= input->number_of_max_cars;
				
				for(i = 0; i < input->number_of_max_cars; i++) {
					
					my_pthread_cond_signal(&blue_traffic);
				}
			}
			else{
				
				temp = blue_queue;
				blue_queue = 0;
				
				for(i = 0; i < temp; i++) {
					
					my_pthread_cond_signal(&blue_traffic);
				}
			}
			how_many_red_cars--;
			my_pthread_mutex_unlock(&mtx);
			return;
		}
	}
		
	how_many_red_cars --;
	my_pthread_mutex_unlock(&mtx);
	return;
}

/* Monitor routine for when a blue car tries to enter the bridge. */
void blue_car_enters_bridge(car_T* input) {
	
	my_pthread_mutex_lock(&mtx);
	
	how_many_blue_cars ++;
	
	if(how_many_blue_cars > input->number_of_max_cars) {
		
		blue_queue ++;
		
		my_pthread_cond_wait(&blue_traffic,&mtx); 
		
	}
	
	if(counter_red > 0) {
		
		blue_queue++;
		
		my_pthread_cond_wait(&blue_traffic,&mtx);
	}
	
	counter_blue++;
	
	my_pthread_mutex_unlock(&mtx);
	
	return;
}

/* Monitor routine for when blue car exits the bridge. */
void blue_car_exits(car_T *input) {
	
	int i;
	int temp;
	
	my_pthread_mutex_lock(&mtx);
	
	counter_blue --;
	
	if(counter_blue == 0) {
		
		if(how_many_blue_cars > 0 && how_many_red_cars ==0) {
			
			if(blue_queue > input->number_of_max_cars) {
				
				blue_queue -= input->number_of_max_cars;
				
				for(i = 0; i < input->number_of_max_cars; i++) {
					
					my_pthread_cond_signal(&blue_traffic);
				}
			}
			else{
				
				temp = blue_queue;
				blue_queue = 0;
				
				for(i=0; i < temp; i++) {
					
					my_pthread_cond_signal(&blue_traffic);
				}
			}
			how_many_blue_cars --;
			my_pthread_mutex_unlock(&mtx);
			return;
		}
		 
		else if(how_many_red_cars > 0) {
			
			if(red_queue > input->number_of_max_cars) {
				
				red_queue -= input->number_of_max_cars;
				
				for(i = 0; i < input->number_of_max_cars; i++) {
					
					my_pthread_cond_signal(&red_traffic);
				}
			}
			else{
				
				temp = red_queue;
				red_queue = 0;
				
				for(i = 0; i < temp; i++) {
					
					my_pthread_cond_signal(&red_traffic);
				}
			}
			how_many_blue_cars--;
			my_pthread_mutex_unlock(&mtx);
			return;
		}
	}
		
	how_many_blue_cars --;
	my_pthread_mutex_unlock(&mtx);
	return;
}

/* The blue car thread(s). */
void * blue_car(void * arg) {

	car_T* input = (car_T*) arg;
	
	blue_car_enters_bridge(input);
	
	printf("Blue car (thread): %lu, n0: %d ,enters the bridge\n",pthread_self(),counter_blue);
//	sleep(input->sleep_time);
	
	blue_car_exits(input);
	
	return NULL;
}

/* The red car thread(s). */
void * red_car(void * arg) {

	car_T* input = (car_T*) arg;
	
	red_car_enters_bridge(input);
	
	printf("Red car (thread): %lu, n0: %d ,enters the bridge\n",pthread_self(),counter_red);
//	sleep(input->sleep_time);
	
	red_car_exits(input);
	
	return NULL;
}

int main (int argc,char* argv[]) {
	
	char input_str[MAX_CARS];
	int index = 0;
	int check;
	int i;
	pthread_t cars_id[MAX_CARS];
	car_T arguments;
	
	if (argc != 3) {
		fprintf(stderr, "Wrong number of arguments!\n");
		return (1);
	}
	arguments.number_of_max_cars = atoi(argv[1]);
	arguments.sleep_time = atoi(argv[2]);
	
	fgets(input_str, MAX_CARS, stdin);
	
	
	while(input_str[index] != '\0') {

		if (input_str[index] == 'b') {
		//	sleep(arguments.sleep_time -1);
			check = pthread_create(&cars_id[index], NULL, blue_car, &arguments);
			if (check != 0) {
				fprintf(stderr, "You fucked up! (line %d)\n", __LINE__);
				exit(EXIT_FAILURE);
			}
		}
		else if (input_str[index] == 'r'){
	//		sleep(arguments.sleep_time - 1);
			check = pthread_create(&cars_id[index], NULL, red_car,&arguments);
			if (check != 0) {
				fprintf(stderr, "You fucked up! (line %d)\n", __LINE__);
				exit(EXIT_FAILURE);
			}
		}
		index++;
	}
	
	for(i = 0; i < (index -1); i++) {
		
		pthread_join(cars_id[i],NULL);
	}
	
	return 0;
}
