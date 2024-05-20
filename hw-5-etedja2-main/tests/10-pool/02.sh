#!/bin/bash

# check that Test 9 and 10 enabled
# if this fails, print error and exit
STR="Please provide the number of threads to use\nTest 9 and 10 enabled"
if ./hw5  | grep -q "$STR"; then
    echo "ERROR: Test 9 and 10 not enabled"
    exit 1
fi

# check tests 05-08
grep "TEST 05-file PASSING" results
grep "TEST 06-ping-echo-write-read-file PASSING" results
grep "TEST 07-write-write-write PASSING" results
grep "TEST 08-stats PASSING" results