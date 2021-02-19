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

/* Useful global variables that work towards concurrency
 * when using threads */
extern char* start;	/* The start of the pipe */
extern char* end;	/* The end of the pipe */
extern char* write_ptr;/* The pointer indicating the "position" of the write end of the pipe */
extern char* read_ptr;	/* The pointer indicating the "position" of the read end of the pipe */
extern int end_write;	/* end_write == 1 when there's nothing else to write in pipe (indicates termination of write */

void pipe_init(int size) {
    
    start = (char*)malloc(sizeof(char)*size);
    if (start == NULL) {
	fprintf(stderr, "Error in malloc in pipe_init,errno: %d,line: %d\n", errno,__LINE__);
	_exit(EXIT_FAILURE);
    }
    end = start + size;
    write_ptr = start;	/* Both write and read end start from the beginning of the pipe. */
    read_ptr = start;
    end_write = 0;
    return;
}

void pipe_close() {
    end_write = 1;	/* Indicating that write end will close. */
    return;
}

int pipe_write(char input) {
    /* Active wait condition: pipe is full. */
    *write_ptr = input;
	
    if (write_ptr == end) {
		write_ptr = start;
		return(1);
    }
    
	write_ptr += 1;
	
	return(0);
}

int pipe_read(char* byte_to_read) {
    
    fputc(*byte_to_read, stdout);
    
    /* Wraping around, thus catching up to write end. lap around = 0. */
    if (read_ptr == end) {
		read_ptr = start;
		return(1);
	}
	read_ptr += 1;
    
    return (0);
}


/* END OF LIBRARY FILE */
