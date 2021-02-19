/* 
 * Tsivgiouras Kon/nos, AEM: 2378 (ktsivgiouras@uth.gr)
 * Koffas Georgios, AEM: 2389 (gkoffas@uth.gr)
 * 
 * User-level thread API, that also features scheduling via use of signal handling.
 * 
 * Friday, 21/12/2018
 * University of Thessaly, Department of ELectrical & Computer Engineering
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

/* Max size for semaphore queues. */
#define MAX_QUEUE_SIZE 100

/* Alarm specifications. */
#define SECS 0
#define USECS 100

/* Error handling. */
#define ERR_PRINT(msg) \
	fprintf(stderr, "In line %d:", __LINE__); \
	perror(msg); \
	_exit(EXIT_FAILURE);


/* User-level semaphore struct. */
struct sem_t {
	
	int val;	/* Semapore value. */
	int queue[MAX_QUEUE_SIZE];	/* Queue for blocked user-level threads. */
	int fifo_turn;	/* The position of new blocked thread in queue. */
	
};

typedef struct sem_t sem_t;

/* User-level thread struct. */
struct my_thread {
	
	ucontext_t self;	/* Used for the context setting/swiching. */
	int blocked;	/* Used by scheduler to determine if signal is blocked. */
	int terminate;	/* Used by join() to determine if signal has terminated. */
	struct my_thread *join;	/* The thread that uses join() on current thread. */
	unsigned long int id;	/* Thread ID. */
	char stack[SIGSTKSZ];	/* Stack used by thread. */
	struct my_thread *next;	/* Link for next thread in thread list */
	
};

typedef struct my_thread thr_t;


extern thr_t *head;	/* Head of the thread list. Points to main thread. */
extern thr_t *scheduler_p;	/* Points to current thread running. Changed from scheduler. */
ucontext_t sceduler_c;	/* Context of scheduler. */
char sceduler_stack[SIGSTKSZ];	/* Scheduler's stack. */
extern struct sigaction act_def;	/* Default action for SIGALRM (=> send to handler) */
extern struct sigaction act_ign;	/* Action to ignore SIGALRM. */
int just_terminate;	/* Flag for knowing if thread that just entered scheduler has terminated. */

/* Yield() function for user-level threads: just raises a SIGALRM. */
int mythreads_yield() {
	
	if (raise(SIGALRM) != 0) {
		ERR_PRINT("\n\tError in raise:")
	}
	
	return(0);
}

/* When threads are destoryed, they call this function. */
void call_yield ()  {
	
	mythreads_yield();
	
}

/* The scheduler (also works as the signal handler). */
void handler(int signum,siginfo_t *info,void *context) {
	
	/* Reset the alarm. */
	ualarm(USECS,0);
	//printf("scheduler_p called : %lu\n",scheduler_p->id);
	
	thr_t *temp = scheduler_p;
	
	//(void)write(STDOUT_FILENO, "Time out!\n", 10);
	
	/* Round-robin policy. */
	scheduler_p = scheduler_p->next;
	
	while(scheduler_p->blocked == 1) {
		scheduler_p = scheduler_p->next;
	}
	
	//printf("change context\n");
	
	//printf("going to scheduler_p : %lu\n",scheduler_p->id);
	
	//fprintf(stderr,"%d",scheduler_p->self);
	
	//if (setitimer(ITIMER_REAL, &tval, &tval) != 0) {
	//	ERR_PRINT("\n\tError in setititmer:")
	//}
	
	swapcontext(&(temp->self),&(scheduler_p->self));
	
}

/* Init execution environment. */
int mythreads_init(ucontext_t main_c) {
	
	srand(time(NULL));
	
 	if ((head = malloc(sizeof(thr_t))) == NULL) {
 		ERR_PRINT("\n\tError in malloc:")
 	}
 	just_terminate = 0;
	head->self = main_c;
	head->next = head;
	head->blocked = 0;
	head->terminate = 0;
	head->join = NULL;
	head->id = rand() % RAND_MAX + (RAND_MAX/2);
	head->self.uc_stack.ss_sp = head->stack;
	head->self.uc_stack.ss_size = sizeof(head->stack );
	head->self.uc_link = NULL;
	
	
	scheduler_p = head;	/* Points to main thread in beginning. */
	
	
	sceduler_c.uc_stack.ss_sp= sceduler_stack;
	sceduler_c.uc_stack.ss_size = sizeof(sceduler_stack);
	sceduler_c.uc_link = &(head->self);
	getcontext(&sceduler_c);
	makecontext(&sceduler_c,call_yield,0);
	
	
	act_def.sa_sigaction = handler;
	act_ign.sa_handler = SIG_IGN;
	act_def.sa_flags = SA_RESTART | SA_SIGINFO;	/* We use RTS for easier scheduling */
	act_ign.sa_flags = SA_RESTART;
	if (sigaction(SIGALRM, &act_def, NULL) != 0) {
		ERR_PRINT("\n\tError in sigaction:")
	}
	
	//printf("head id: %lu\n",head->id);
	
	mythreads_yield();	/* Used to start the program timer. */
	
	return 0;
}

/* Function to create a new thread and add it to the thread list. */
int mythreads_create(thr_t *thread, void(*body)(void), void *arg) {
	
	thr_t *temp;
	
	/* We need thread creation to not be interrupted by any signals. */
	if (sigaction(SIGALRM, &act_ign, NULL) != 0) {
		ERR_PRINT("\n\tError in sigaction:")
	}
	
	thread->blocked = 0;
	thread->terminate = 0;
	thread->join = NULL;
	thread->id = rand() % RAND_MAX + (RAND_MAX / 2);
	thread->self.uc_stack.ss_sp = thread->stack;
	thread->self.uc_stack.ss_size = sizeof(thread->stack);
	temp = head->next;
	head->next = thread;
	thread->next = temp;
	thread->self.uc_link = &sceduler_c;
	
	if (getcontext(&(thread->self)) != 0) {
 		ERR_PRINT("\n\tError in getcontext:")
 	}
	makecontext(&(thread->self), body, 1, arg);
	
	//fprintf(stderr,"thread created, id = %lu\n",thread->id);
	
	if (sigaction(SIGALRM, &act_def, NULL) != 0) {
		ERR_PRINT("\n\tError in sigaction:")
	}
	
	return(0);
}

/* Semaphore initialization. */
int mythreads_sem_init(sem_t *s,int val) {
	
	s->val = val;
	s->fifo_turn = 0;
	return(0);
}

/* DEcrement of semaphore by 1. If new value is negative, thread using that semaphore is blocked. */
int mythreads_sem_down(sem_t *s) {
	
	if (sigaction(SIGALRM, &act_ign, NULL) != 0) {
		ERR_PRINT("\n\tError in sigaction:")
	}
	
	s->val--;
	
	if(s->val < 0) {
		scheduler_p->blocked = 1;
		s->queue[s->fifo_turn] = scheduler_p->id;
		s->fifo_turn++;
		
		if (sigaction(SIGALRM, &act_def, NULL) != 0) {
			ERR_PRINT("\n\tError in sigaction:")
		}
		if (raise(SIGALRM) != 0) {
			ERR_PRINT("\n\tError in raise:")
		}
	}
	if (sigaction(SIGALRM, &act_def, NULL) != 0) {
		ERR_PRINT("\n\tError in sigaction:")
	}
	return(0);
}

/* Increment semaphore by 1. If semaphore was non-positive, unblock thread using it. */
int mythreads_sem_up(sem_t *s) {
	
	thr_t *temp = head;
	int i;
	
	if (sigaction(SIGALRM, &act_ign, NULL) != 0) {
		ERR_PRINT("\n\tError in sigaction:")
	}
	s->val ++;
	
	if(s->val <= 0) {
		
		while(temp->id != s->queue[0]) {
			
			temp = temp->next;
		}
		
		temp->blocked = 0;
		
		for(i=0; i < MAX_QUEUE_SIZE; i++) {
			s->queue[i] = s->queue[i+1];
		}
		s->fifo_turn --;
	}
	if (sigaction(SIGALRM, &act_def, NULL) != 0) {
		ERR_PRINT("\n\tError in sigaction:")
	}
	
	return(0);
}

/* NOT USED IN THIS VERSION OF THE API */
// int mythreads_sem_destroy(sem_t *s) {
// 	
// 	free(s);
// 	
// 	return 0;
// }

/* Join() function that waits for thread to finish. */
int mythreads_join(thr_t *thread) {
	
	if(thread->terminate == 0) {
		
		thread->join = scheduler_p;
		scheduler_p->blocked = 1;
		thread->terminate = 1;
		mythreads_yield();
	}
	
	
	return 0;
}

/* Function that removes all links of thread frm the thread list, making it "invisible" by scheduler, thus deeming it terminated. */
int mythreads_destroy(thr_t *thread) {
	
	thr_t *temp = head;
	
	if (sigaction(SIGALRM, &act_ign, NULL) != 0) {
		ERR_PRINT("\n\tError in sigaction:")
	}
	
	while(temp->next != thread) {
		
		temp = temp->next;
	}
	
	temp->next = thread->next;

	
	if(thread->terminate == 1) {
		thread->join->blocked = 0;
	}
	else {
		
		thread->terminate = 1;
	}
	
	//fprintf(stderr,"thread %lu deleted\n",thread->id);
	
	if (sigaction(SIGALRM, &act_def, NULL) != 0) {
		ERR_PRINT("\n\tError in sigaction:")
	}
	
	return(0);
}
