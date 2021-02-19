/* 
 * Koffas Georgios, AEM: 2389 (gkoffas@uth.gr)
 * Tsivgiouras Kon/nos, AEM: 2378 (ktsivgiouras@uth.gr)
 * 
 * A program testing concurrency by the use of threads through the
 * implementation of a 1-way FIFO pipe between two threads. Uses functions defined
 * in the cbuf_pipe.h file. Program is run like this:
 * 	./thread_application (name of input file) > (name of output file)
 * 
 * Thursday, 11/10/2018
 * University of Thessaly, Department of Electrical & Computer Engineering
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "cbuf_pipe.h"

/* Default size of pipe. Can be changed as seen fit. */
#define DEF_SIZE 250

volatile char* read_ptr;	/* Stores the "position" of read end of pipe, is only edited by pipe_read(). */
volatile char* write_ptr;	/* Stores the "position" of write end of pipe, is only edited by pipe_write(). */
volatile char* start;		/* Stores the start "position" of the pipe, only read. */
volatile char* end;		/* Stores the end "position" of the pipe, only read. */
volatile int lap_around;	/* Determines whether write has circled around read: if lap_around == 1, write has circled around,
				    if lap_around == 0, read has caught up. */ 
volatile int end_write;		/* == 1 when write has no more bytes to store in buffer. */
volatile int end_read;		/* == 1 when read has no more bytes to read (and subsequently write to output file). */
volatile FILE* input_file_d;	/* A pointer to store the input file. */


/* The thread function that writes in the pipe. */ 
void * write_thread(void *arg) {
    /* The use of unsigned char is essential: while we were debugging the program,
     * we noticed that for some reason, the output file always ended up stoping before
     * having the FF byte written to it. That byte has a value of 255, so we tried using
     * unsigned char to store the byte we write to the pipe. However, that prevented us from
     * checking for EOF (since it's value is -1!!). So we ended up doing an extra check with fgetc,
     * and to reposition the file pointer we used fseek. */
    unsigned char temp;
    
    temp = fgetc((FILE *)input_file_d);
    while(1) {
	if (fgetc((FILE *)input_file_d) == EOF) {	/* Termination condition. */
	    pipe_write(temp);
	    break;
	}
	pipe_write(temp);
	fseek((FILE *)input_file_d, -1, SEEK_CUR);	/* Repositioning the file pointer. */
	temp = fgetc((FILE *)input_file_d);
    }
    pipe_close();	/* Signaling main that we've finished */
    return NULL;
}

/* The thread function that reads from the pipe and outputs to stdout (which
 * is redirected to a file of our choice). */
void * read_thread(void *arg) {
    
    while(1) {
	if (read_ptr == write_ptr && end_write == 1) {	/* Termination condition. */
	    break;
	}
	pipe_read((char*)read_ptr);
    }
    
    end_read = 1;	/* Signaling main that we've finished. */
    
    return NULL;
}

int main(int argc, char* argv[]) {
    
    int check;
    int thread_check;
    pthread_t w_thread;
    pthread_t r_thread;
    
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
    
    thread_check = pthread_create(&w_thread, NULL, write_thread, NULL);
    if (thread_check != 0) {
	fprintf(stderr, "Thread creation failure. Error code: %d\n", errno);
	_exit(EXIT_FAILURE);
    }
    
    thread_check = pthread_create(&r_thread, NULL, read_thread, NULL);
    if (thread_check != 0) {
	fprintf(stderr, "Thread creation failure. Error code: %d\n", errno);
	_exit(EXIT_FAILURE);
    }
    
    /* Active wait for the write/read threads to terminate. */
    while(1) {
	if (end_write == 1 && end_read == 1) {	/* Termination condition. */
	    end = NULL;
	    write_ptr = NULL;
	    read_ptr = NULL;
	    free((void*)start);	/* Don't forget to free all allocated memory! */
	    break;
	}
    }
    
    check = fclose((FILE *)input_file_d);	/* And to close the file. */
    if (check == EOF) {
	fprintf(stderr, "Error in fclose. Error code: %d\n", errno);
	_exit(EXIT_FAILURE);
    }
    
    return 0;
}
