#!/bin/bash

if (($# < 5 || $# > 6)); then
	echo "Not enough arguments!"
	exit 1
else
	PROGRAM_NAME=$(readlink -f $1)
	WORKERS_NUM=$2
	INPUT_FILE=$3
	OUTPUT_FILE=$4
	LOG_FILE=$5
fi

((FIRST_TIME=1))
((MAX_WORKERS = 150))

while [ $WORKERS_NUM -le $MAX_WORKERS ]
do
	if ((FIRST_TIME == 1)); then
		echo "Working with: $WORKERS_NUM threads"
		{ time $PROGRAM_NAME $WORKERS_NUM < $INPUT_FILE > $OUTPUT_FILE ; } 2> $LOG_FILE && vim $4
		((FIRST_TIME = 0))
		sleep 1
	else
		echo "Working with: $WORKERS_NUM threads"
		{ time $PROGRAM_NAME $WORKERS_NUM < $INPUT_FILE > $OUTPUT_FILE ; } 2>> $LOG_FILE && vim $4
		sleep 1
	fi
	((WORKERS_NUM = WORKERS_NUM + 1))
done
