#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "ccr.h"


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

CCR_DECLARE(bridge)
CCR_INIT(bridge)

void * blue_car(void * arg) {

	car_T* input = (car_T*) arg;
	
	CCR_EXEC(bridge,((counter_blue < (input->number_of_max_cars)) && (counter_red == 0)),counter_blue ++;)
	
	printf("Blue car (thread): %lu, n0: %d ,enters the bridge %d\n",pthread_self(),counter_blue,counter_red);
	sleep(input->sleep_time);
	
	CCR_EXEC(bridge,1,counter_blue--;)
	
	return NULL;
}

void * red_car(void * arg) {

	car_T* input = (car_T*) arg;
	
	CCR_EXEC(bridge,((counter_red < (input->number_of_max_cars)) && (counter_blue == 0)), counter_red++;)
	
	printf("Red car (thread): %lu, n0: %d ,enters the bridge %d\n",pthread_self(),counter_red,counter_blue);
	sleep(input->sleep_time);
	
	
	CCR_EXEC(bridge,1,counter_red--;)
	
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
			sleep(arguments.sleep_time -1);
			check = pthread_create(&cars_id[index], NULL, blue_car, &arguments);
			if (check != 0) {
				fprintf(stderr, "You fucked up! (line %d)\n", __LINE__);
				exit(EXIT_FAILURE);
			}
		}
		else if (input_str[index] == 'r'){
		sleep(arguments.sleep_time - 1);
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


