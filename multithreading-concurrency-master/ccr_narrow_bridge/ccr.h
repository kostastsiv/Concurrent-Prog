#include <stdio.h>
#include <pthread.h>

/* Declaring the shared variables. */
#define CCR_DECLARE(label) \
	pthread_mutex_t mtx_##label; \
	pthread_cond_t cond_q1_##label; \
	volatile int q1_count_##label;\
	volatile int q1_free_##label;\
	int first_time_##label;

/* Initialising shared variables. */
#define CCR_INIT(label)\
	volatile int q1_count_##label = 0;\
	volatile int q1_free_##label = 1;

/* Executing under CCR (using only 1 queue). */
#define CCR_EXEC(label, condition, body) \
	first_time_##label = 1;\
	pthread_mutex_lock(&(mtx_##label)); \
	q1_count_##label ++;\
	while(!condition) {\
		if(first_time_##label == 1) {\
			first_time_##label = 0;\
			pthread_cond_wait(&(cond_q1_##label),&(mtx_##label));\
			continue;\
		}\
		if(q1_free_##label != q1_count_##label){\
			q1_free_##label++;\
			pthread_cond_signal(&(cond_q1_##label));\
		}\
		else{\
			q1_free_##label = 1;\
		}\
		pthread_cond_wait(&(cond_q1_##label),&(mtx_##label));\
	}\
	q1_count_##label --;\
	body\
	pthread_cond_signal(&(cond_q1_##label));\
	pthread_mutex_unlock(&(mtx_##label)); \
	
	
