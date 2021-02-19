#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "bsem.h"

#define MAX_CARS 500
#define MAX_LIMIT 50

volatile int counter_blue = 0;	/* Shows how many blue cars are on bridge */
volatile int counter_red = 0;	/* Shows how many red cars are on bridge */
volatile int traffic_red;	/* Semaphore indicating whether there's red cars on bridge */
volatile int traffic_blue;	/* Semaphore indicating whether there's blue cars on bridge */
volatile int limit_of_red_cars;
volatile int limit_of_blue_cars;
volatile int cs;
volatile int cs_blue;
volatile int cs_red;
volatile int how_many_red_cars = 0;
volatile int how_many_blue_cars = 0;
volatile int red_cars_waiting = 0;
volatile int blue_cars_waiting = 0;
volatile char turn;
volatile int index_gl_b = 0;
volatile int index_gl_r = 0;
volatile int i_gl = 0;
volatile int terminate_sig;

struct car {
	int sleep_time;
	int number_of_max_cars;
};

typedef struct car car_T;


int limit_sem_blue[MAX_LIMIT];
int limit_sem_red[MAX_LIMIT];
int blue_log[MAX_LIMIT];
int red_log[MAX_LIMIT];

void critical_mem (int sem, int * var, int flag) {
	mysem_down(sem);
	switch (flag) {
		case 0:
			*var += 1;
			break;
		case 1:
			*var -= 1;
			break;
		default:
			break;
	}
	mysem_up(sem);
	return;
}

void * blue_car(void * arg) {

	int temp;
	int i;
	car_T* input = (car_T*) arg;
	int counter = 0;
	
	mysem_down(cs);
	how_many_blue_cars ++;
	temp = index_gl_b;
	index_gl_b++;
	
	if(index_gl_b == input->number_of_max_cars) {
		index_gl_b = 0;
	}	
	mysem_up(cs);
	
	mysem_down(limit_sem_blue[temp]);
	
	
	i_gl++;
	printf("Blue car n:%d enters the bridge i =%d \n",counter_blue,i_gl);
	sleep(input->sleep_time);

	
	mysem_down(cs);
	
	how_many_blue_cars --;
	
	printf("blue log: %d\n",temp);
	blue_log[temp] = 1;
	
	for(i = 0; i < input->number_of_max_cars; i++) {
		
		if(blue_log[i] == 1) {
			counter++;
		}
	}
	
	//printf("how many blue:%d\n",how_many_blue_cars);
	
	if(how_many_blue_cars == 0 || counter == input->number_of_max_cars ) {
		
		for(i = 0; i < input->number_of_max_cars; i++) {
			
			if(blue_log[i] == -1) {
				printf("aaa b %d\n",i);
				mysem_down(limit_sem_blue[i]);
				
			}
			else{
				blue_log[i] = -1;
			}
		}
		
		if(how_many_red_cars == 0 && how_many_blue_cars > 0) {
			
			for(i = 0; i < input->number_of_max_cars; i++) {
				//printf("eeeee\n");
				mysem_up(limit_sem_blue[i]);
			}
		}
		else if(how_many_red_cars > 0) {
			
			for(i = 0; i < input->number_of_max_cars; i++) {
	//			printf("aaaeee\n");
				mysem_up(limit_sem_red[i]);
			}
		}
		else if(how_many_blue_cars == 0 && how_many_red_cars == 0) {
			mysem_up(terminate_sig);
		}
	}
	mysem_up(cs);
	return NULL;
}

void * red_car(void * arg) {

	int temp;
	int i;
	car_T* input = (car_T*) arg;
	int counter = 0;
	
	
	mysem_down(cs);
	how_many_red_cars ++;
	temp = index_gl_r;
	index_gl_r++;
	
	if(index_gl_r == input->number_of_max_cars) {
		index_gl_r = 0;
	}	
	mysem_up(cs);

	mysem_down(limit_sem_red[temp]);
	
	i_gl++;
	printf("Red car n:%d enters the bridge i =%d \n",counter_red,i_gl);
	sleep(input->sleep_time);
	

	mysem_down(cs);
	
	printf("red log write %d\n",temp);
	red_log[temp] = 1;
	
	how_many_red_cars --;
	
	for(i = 0; i < input->number_of_max_cars; i++) {
		
		if(red_log[i] == 1) {
			counter++;
		}
	}
	
	//printf("how many reds:%d\n",how_many_red_cars);
	
	if(how_many_red_cars == 0 || counter == input->number_of_max_cars) {
		
		for(i = 0; i < input->number_of_max_cars; i++) {
			
			if(red_log[i] == -1) {
				printf("aaa %d\n",i);
				mysem_down(limit_sem_red[i]);
				
			}
			else {
				red_log[i] = -1;
			}
		}
		
		if(how_many_blue_cars == 0 && how_many_red_cars > 0) {
			
			for(i = 0; i < input->number_of_max_cars; i++) {
			//	printf("aaaaaaa\n");
				mysem_up(limit_sem_red[i]);
				
			}
		}
		else if(how_many_blue_cars > 0) {
			
			//printf("eeee\n");
			for(i = 0; i < input->number_of_max_cars; i++) {
				mysem_up(limit_sem_blue[i]);
			}
		}
		else if(how_many_blue_cars == 0 && how_many_red_cars == 0) {
			mysem_up(terminate_sig);
		}
	}
	
	mysem_up(cs);
	return NULL;
}
int main (int argc,char* argv[]) {
	
	char input_str[MAX_CARS];
	int index = 0;
	int check;
	int i;
	pthread_t cars_id[MAX_CARS];
	int number_of_max_cars;
	int sleep_time;
	int temp_table[MAX_CARS];
	car_T arguments;
	
	
	
	if (argc != 3) {
		fprintf(stderr, "Wrong number of arguments!\n");
		return (1);
	}
	arguments.number_of_max_cars = atoi(argv[1]);
	arguments.sleep_time = atoi(argv[2]);
	
	fgets(input_str, MAX_CARS, stdin);
	
	for(i = 0; i < arguments.number_of_max_cars; i++) {
		
		blue_log[i] = -1;
		red_log[i] = -1;
		
	}
	
	terminate_sig = mysem_create(0);
	traffic_blue = mysem_create(0);
	traffic_red = mysem_create(0);
	limit_of_red_cars = mysem_create(0);
	limit_of_blue_cars = mysem_create(0);
	cs = mysem_create(1);
	
	
	while(input_str[index] != '\0') {
		
		if(index == 0) {
			
			if(input_str[index] == 'b') {
				for(i=0; i < arguments.number_of_max_cars; i++) {
					limit_sem_blue[i] = mysem_create(1);
					limit_sem_red[i] = mysem_create(0);
				}
			}
			else if(input_str[index] == 'r'){
				for(i=0; i < arguments.number_of_max_cars; i++) {
					limit_sem_blue[i] = mysem_create(0);
					limit_sem_red[i] = mysem_create(1);
				}
			}
		}
		
		if (input_str[index] == 'b') {
			sleep((arguments.sleep_time) - 1);
			check = pthread_create(&cars_id[index], NULL, blue_car, &arguments);
			if (check != 0) {
				fprintf(stderr, "You fucked up! (line %d)\n", __LINE__);
				exit(EXIT_FAILURE);
			}
		}
		else if (input_str[index] == 'r'){
			sleep((arguments.sleep_time) - 1);
			check = pthread_create(&cars_id[index], NULL, red_car,&arguments);
			if (check != 0) {
				fprintf(stderr, "You fucked up! (line %d)\n", __LINE__);
				exit(EXIT_FAILURE);
			}
		}
		index++;
	}
	
	mysem_down(terminate_sig);
	
	
	return 0;
}
