#!/bin/bash

# shell command: ./test_thread.sh "<name of input file>" "<name of output file>"
# also don't forget to write the command chmod a+rx test_thread.sh before running it

PROGRAM_NAME=$(readlink -f $1)
INPUT_FILE=$2
OUTPUT_FILE=$3
ITERATIONS=0


((MAX_ITERS = 10**2))


while [ $ITERATIONS -lt $MAX_ITERS ]
do
    $PROGRAM_NAME $2 > $3
    result=$(diff $2 $3)
     
    if ((result == 0)); then
	if ((ITERATIONS == 0)); then
	    echo "No differencies found!" > log.txt
	else echo "No differencies found!" >> log.txt
	fi
    elif ((result == 1)); then
	if ((ITERATIONS == 0)); then
	    echo "The files are different!" > log.txt
	else echo "The files are different!" >> log.txt
	fi
    else
	if ((ITERATIONS == 0)); then
	    echo "Oops! An error occured!" > log.txt
	else echo "The files are different!" >> log.txt
	fi
    fi
    ((ITERATIONS = ITERATIONS + 1))
done
