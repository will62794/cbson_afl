#!/bin/bash

# A simple script that runs the BSON fuzzing harness with all testcases in the "findings/crashes" 
# directory, for simple analysis of bug classes. May serve as a first step in a more thorough 
# crash diagnosis.

FILES="findings/crashes/id*"
for f in $FILES
do
    printf "[ TESTCASE ] - %s\n" $f
    echo  "Hexdump:"
    hexdump -C $f
    echo "----"
    ./fuzz_bson < $f
    printf "Exit code: %s\n\n\n" $?
done