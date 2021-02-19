/* 
 * Tsivgiouras Kon/nos, AEM: 2378 (ktsivgiouras@uth.gr)
 * Koffas Georgios, AEM: 2389 (gkoffas@uth.gr)
 * 
 * Program that utilizes out coroutine API to build a pipe for file transfering.
 * 
 * Friday, 21/12/2018
 * University of Thessaly, Department of Electrical & Computer Engineering
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <ucontext.h>
#include <signal.h>
#include "cbuf_pipe.h"
#include "mycoroutines.h"

/* Default size of pipe. Can be changed as seen fit. */
#define DEF_SIZE 512

char* read_ptr;
char* write_ptr;
char* start;
char* end;
int lap_around;
int end_write;	
FILE* input_file_d;	/* A pointer to store the input file. */
co_t write_corout;	/* Coroutine for write end of pipe. */
co_t read_corout;	/* Coroutine for read end of pipe. */

 
void write_co(void* arg) {
	
    unsigned char temp;
    int switch_to;
    temp = fgetc(input_file_d);
    while(1) {
		
		if (fgetc(input_file_d) == EOF) {	/* Termination condition. */
			pipe_write(temp);
			break;
		}
		switch_to = pipe_write(temp);
		fseek(input_file_d, -1, SEEK_CUR);	/* Repositioning the file pointer. */
		temp = fgetc(input_file_d);
		
		if(switch_to == 1) {
			mycoroutines_switchto(&read_corout);
		}
	}
    pipe_close();	/* Signaling main that we've finished */
}


void read_co(void *arg) {
  
	int check;
    while(1) {
		
		if (read_ptr == write_ptr && end_write == 1) {	/* Termination condition. */
			break;
		}
		check = pipe_read(read_ptr);
		if(check ==1) {
			mycoroutines_switchto(&write_corout);
		}
    }
}

int main(int argc, char* argv[]) {
    
    int check;
	co_t main_corout;
	char stack_write[SIGSTKSZ];	/* Stack for write coroutine. */
	char stack_read[SIGSTKSZ];	/* Stack for read coroutine. */
	
	
    switch(argc) {
	case 1:
	    fprintf(stderr, "Too little command line arguments!\n");
	    _exit(EXIT_FAILURE);
	case 2:
	    pipe_init(DEF_SIZE);
	    input_file_d = fopen(argv[1], "r");
	    if (input_file_d == NULL) {
			fprintf(stderr, "Error in fopen. Error code: %d\n", errno);
			_exit(EXIT_FAILURE);
	    }
	    break;
	default:
	    fprintf(stderr, "Too many command line arguments!\n");
	    _exit(EXIT_FAILURE);
    }
	
	/* Initializing coroutines. */
	write_corout.uc_stack.ss_sp = stack_write;
	write_corout.uc_stack.ss_size = sizeof(stack_write);
	write_corout.uc_link = &(read_corout);
	
	read_corout.uc_stack.ss_sp = stack_read;
	read_corout.uc_stack.ss_size = sizeof(stack_read);
	read_corout.uc_link = &(main_corout);
	
	/* Setting up main coroutine. */
    mycoroutines_init(&main_corout);
	
	
	if(end_write == 0) {	/* When we jump back to main coroutine, we want transfer to not have ended. */
		mycoroutines_create(&write_corout, write_co,NULL);
		mycoroutines_create(&read_corout, read_co, NULL);
		mycoroutines_switchto(&write_corout);

	}
	
    check = fclose(input_file_d);	/* Closing file when done. */
    if (check == EOF) {
		fprintf(stderr, "Error in fclose. Error code: %d\n", errno);
		exit(EXIT_FAILURE);
    }
    
    mycoroutines_destroy(&write_corout);
	mycoroutines_destroy(&read_corout);
    
    return 0;
}
