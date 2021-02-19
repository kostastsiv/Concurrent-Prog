/* Tsivgiouras Kon/nos, AEM: 2378 (ktsivgiouras@uth.gr)
 * Koffas Georgios, AEM: 2389 (gkoffas@uth.gr)
 * 
 * The following is an implementation of the quick sort algorithm
 * using recursion and multithreading.
 * 
 * Tuesday, 16/10/2018
 * University of Thessaly, Department of Electrical & Computer Engineering
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

/* Max size of integers to be sorted. Can be changed anytime. */
#define MAX_SIZE 200

/* The argument passed to the recursive threads. */
struct limits {
	
	int low;	/* The left border of the algorithm. */
	int high;	/* The reight border of the algorithm. */
	int finish;	/* The indication that a thread has finished with the sorting. */
	struct limits* next_1;	/* Used to identify the order */
	struct limits* next_2;	/* in which the threads were created. */
};

typedef struct limits limits_t;

/* The shared memory array to be sorted. */
int table[MAX_SIZE];


/* Function that creates the partitions of the
 * array to be sorted. */
int partition(int low,int high) {
	
	int pivot = table[high];	/* Pivot = stoixeio diaxwrismou. */
	int i = (low - 1);			/* i and j are used to traverse the array. */
	int j;
	int temp;					/* Used for swapping purposes. */
	
	for(j = low; j<= high - 1; j++) {
		
		if(table[j] <= pivot) {
			i++;
			temp = table[i];
			table[i] = table[j];
			table[j] = temp;
		}
	}
	
	temp = table[i+1];
	table[i+1] = table[high];
	table[high] = temp;
	
	/* Return the point of next recursions. */
	return(i+1);
}

/* Thread function. */
void * qs_thread (void*args) {
	
	limits_t * limits;
	int check;
	pthread_t t1,t2;
	int pi;
	
	limits = (limits_t *)args;
	
	if(limits->low < limits->high) {
		pi = partition(limits->low, limits->high);
	}
	else{
		limits->finish = 1;
		return NULL;
	}
	
	/* Creating a recursion binary tree (list). */
	limits->next_1 = (limits_t*)malloc(sizeof(limits_t));
	
	limits->next_1->high = (pi - 1);
	limits->next_1->low = limits->low;
	limits->next_1->finish = 0;
	
	check = pthread_create(&t1,NULL,qs_thread,limits->next_1);
	
	if(check != 0) {
		
		fprintf(stderr,"error in pthread_create\n");
		_exit(EXIT_FAILURE);
		
	}
	
	limits->next_2 = (limits_t*)malloc(sizeof(limits_t));
	
	limits->next_2->high = limits->high;
	limits->next_2->low = pi + 1;
	limits->next_2->finish = 0;
	
	
	check = pthread_create(&t2,NULL,qs_thread,limits->next_2);
		
	if(check != 0) {
		
		fprintf(stderr,"error in pthread_create\n");
		_exit(EXIT_FAILURE);
	}
	
	/* Waiting for both children threads to finish. */
	while(1) {
		if(limits->next_1->finish == 1 && limits->next_2->finish ==1) {
			break;
		}
	}
	
	limits->finish = 1;
	
	return NULL;
}

int main (int argc,char* argv[]) {
	
	int temp;
	int i = 0;
	pthread_t t1;	/* The root of the recursion tree. */
	int check;
	limits_t* limits;
	
	scanf(" %d",&temp);
	
	while(temp > -1) {
		table[i] = temp;
		scanf(" %d",&temp);
		i++;
	}
	
	table[i] = temp;
	
	limits = (limits_t *)malloc(sizeof(limits_t));
	
	limits->low = 0;
	limits->high = i - 1;
	limits->finish = 0;
	
	check = pthread_create(&t1,NULL,qs_thread,limits);
	
	if(check != 0) {
			fprintf(stderr,"error in pthread_create\n");
			_exit(EXIT_FAILURE);
	}
	
	while(limits->finish == 0) {}
	
	i = 0;
	
	while(table[i] > -1) {
		printf("%d ",table[i]);
		i++;
	}
	putchar('\n');
	return(0);
}
