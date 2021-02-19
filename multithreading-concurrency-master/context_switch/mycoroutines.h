/* 
 * Tsivgiouras Kon/nos, AEM: 2378 (ktsivgiouras@uth.gr)
 * Koffas Georgios, AEM: 2389 (gkoffas@uth.gr)
 * 
 * Coroutine API using the context mechanisms of the ucontext.h library.
 * 
 * Friday 21/12/2018
 * University of Thessaly, Department of Electrical & Computer Engineering
 */

#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

typedef ucontext_t co_t;

/* Saves the current user context. */
co_t *self;

/* Initializing main context, */
int mycoroutines_init(co_t * main) {

	int check;
	
	check = getcontext(main);
	
	if(check != 0) {
		fprintf(stderr,"error in getcontext in init errno: %d ,line: %d\n",errno,__LINE__);
		exit(EXIT_FAILURE);
	}
	
	self = main;
	
	return(check);
}

/* Creates coroutines and their functions. */
int mycoroutines_create(co_t * co, void (body)(void*) ,void*arg) {
	
	int check;
	
	check = getcontext(co);
	
	if(check != 0) {
		fprintf(stderr,"error in getcontext in create errno: %d ,line: %d\n",errno,__LINE__);
		exit(EXIT_FAILURE);
	}
	
	makecontext(co,(void(*)(void))body,1,(void*)arg);
	
	return(check);
}

/* Swaps contexts. */
int mycoroutines_switchto(co_t* co) {
	
	int check;
	co_t *temp = self;
	
	self = co;	/* Updating current context. */
	
	check = swapcontext(temp,co);
	
	if(check == -1) {
		fprintf(stderr,"error in swapcontext in switchto errno: %d,line: %d \n",errno,__LINE__);
		exit(EXIT_FAILURE);
	}
	
	return(check);
}

/* Destroys coroutine links. */
int mycoroutines_destroy(co_t* co) {
	
	co->uc_link = NULL;
	return(0);
}
