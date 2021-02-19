#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>


int mysem_create(int init_value){
    
    int sem_id;
    int retval;
    
    if (init_value < 0 || init_value > 1) {
        fprintf(stderr, "You're an idiot, we're working with binary semaphores here bro!\n");
		exit(EXIT_FAILURE);
    }
    
    sem_id = semget(IPC_PRIVATE,1,IPC_CREAT|0666);
    if(sem_id == -1) {
        fprintf(stderr, "Error in mysem_create: %d, line: %d\n", errno, __LINE__);
        exit(EXIT_FAILURE);
    }
    
    retval = semctl(sem_id, 0, SETVAL, init_value);
    if (retval == -1) {
		fprintf(stderr, "Error in mysem_create: %d, line: %d\n", errno, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	return (sem_id);
}

void mysem_down(int sem_id) {
	
	int retval;
	struct sembuf down_op;
	
	down_op.sem_num = 0;
	down_op.sem_op = -1;
	down_op.sem_flg = 0;
	
	retval = semop(sem_id,&down_op, 1);
	if (retval == -1) {
		if (errno != EIDRM) {
		fprintf(stderr, "Error in mysem_down-semop: %d, line %d\n", errno, __LINE__);
		exit(EXIT_FAILURE);
		}
	}
	
	return;
}

int mysem_up(int sem_id) {
	
	int retval;
	struct sembuf up_op;
	
	up_op.sem_num = 0;
	up_op.sem_op = 1;
	up_op.sem_flg = 0;
	
	retval = semctl(sem_id, 0, GETVAL, NULL);
	if (retval == 1) {
		fprintf(stderr,"Error: extra mysem_up call!\n");
		return(1);
	}
	else if (retval == -1) {
		fprintf(stderr, "Error in mysem_up-semctl: %d, line %d\n", errno, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	retval = semop(sem_id, &up_op, 1);
	if (retval == -1) {
		fprintf(stderr, "Error in mysem_up-semop: %d, line %d\n", errno, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	return 0;
}

void mysem_destroy(int sem_id) {
	
	int retval;
	
	retval = semctl(sem_id, 0, IPC_RMID, NULL);
	if (retval == -1) {
		fprintf(stderr, "Error in mysem_destroy-semctl: %d, line %d\n", errno, __LINE__);
		exit(EXIT_FAILURE);
	}
	
	return;
}
