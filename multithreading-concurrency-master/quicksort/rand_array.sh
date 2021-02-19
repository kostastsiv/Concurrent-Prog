#!/bin/bash

if (($# == 4))
then
	START_VALUE=$1; shift
	MAX_VALUE=$1; shift
	PROGRAM_NAME=$(readlink -f $1); shift
	INPUT_FILE=$1
else
	echo "Wrong number of arguments!"
	exit
fi

VALUE=$START_VALUE
FLAG=1

while [ $VALUE -lt $MAX_VALUE ]
do
	entries=()
	entries=($(shuf -i 0-150 -n $VALUE) "-1")
	for entry in "${entries[@]}"; do
		echo "$entry" >> $INPUT_FILE
	done
	if ((FLAG == 1)); then
		$PROGRAM_NAME < $INPUT_FILE > "out.txt"
		((FLAG = 0))
	else
		$PROGRAM_NAME < $INPUT_FILE >> "out.txt"
	fi
	((VALUE = VALUE + 1))
	truncate -s 0 $INPUT_FILE
done
