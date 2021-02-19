/* 
 * Koffas Georgios, AEM: 2389 (gkoffas@uth.gr)
 * Tsivgiouras Kon/nos, AEM: 2378 ktsivgiouras@uth.gr)
 * 
 * Library that implements pipe functions:
 * 	-void pipe_write(char c): Write a character <c> in the pipe.
 * 	-void pipe_init(int size): Create a pipe of type __char__ and size <size>,
 * 		also initialise pointers used by the pipe_write(), pipe_read() functions,
 * 		and for the traversal of the pipe.
 * 	-void pipe_close(): Close the write end of the pipe.
 * 	-int pipe_read(char* c): Read a character from the memory block <c> is pointing to. Return 0 on success.
 * 
 * Thursday, 11/10/2018
 * University of Thessaly, Department of Electrical and Computer Engineering
 */

/* START OF LIBRARY FILE */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

/* Declaration of the functions of library */
void pipe_init(int size);
void pipe_write(char input);
void pipe_close();
int pipe_read(char* content);

/* Useful global variables that work towards concurrency
 * when using threads */
extern volatile char* start;	/* The start of the pipe */
extern volatile char* end;	/* The end of the pipe */
extern volatile char* write_ptr;/* The pointer indicating the "position" of the write end of the pipe */
extern volatile char* read_ptr;	/* The pointer indicating the "position" of the read end of the pipe */
extern volatile int lap_around;	/* lap_around == 1 if write end has managed to wrap around from the end to the start
					of the pipe before read end, thus being a "lap" ahead. lap_around goes back
					to 0 when read end catches up by wraping around from the end to the start of the pipe */
extern volatile int end_write;	/* end_write == 1 when there's nothing else to write in pipe (indicates termination of write_thread */
extern volatile int end_read;	/* Same as end_write, it's 1 when there's nothing else to read and end_write == 1 */


void pipe_init(int size) {
    
    start = (char*)malloc(sizeof(char)*size);
    if (start == NULL) {
	fprintf(stderr, "Error in malloc: %d\n", errno);
	_exit(EXIT_FAILURE);
    }
    end = start + size;
    write_ptr = start;	/* Both write and read end start from the beginning of the pipe. */
    read_ptr = start;
    lap_around = 0;	/* Since both write and read pointers are just starting. */
    end_write = 0;
    end_read = 0;
    return;
}

void pipe_close() {
    end_write = 1;	/* Indicating that write end will close. */
    return;
}

void pipe_write(char input) {
    /* Active wait condition: pipe is full. */
    while(write_ptr == read_ptr && lap_around == 1) {
	continue;
    }
    
    *write_ptr = input;
    /* Wraping around, thus getting ahead of read end. lap_around = 1. */
    if (write_ptr == end) {
	write_ptr = start;
	lap_around = 1;
    }
    else {
	write_ptr += 1;
    }
}

int pipe_read(char* byte_to_read) {
    
    /* Active wait condition: pipe is empty. */
    while(read_ptr == write_ptr && lap_around == 0) {
	if (end_write == 1) {
	    return 0;
	}
	else continue;
    }
    
    fputc(*byte_to_read, stdout);
    
    /* Wraping around, thus catching up to write end. lap around = 0. */
    if (read_ptr == end) {
	read_ptr = start;
	lap_around = 0;
    }
    else {
	read_ptr += 1;
    }
    
    return 0;
}


/* END OF LIBRARY FILE */
