#!/bin/bash

# check that Test 9 and 10 enabled
# if this fails, print error and exit
STR="Please provide the number of threads to use\nTest 9 and 10 enabled"
if ./hw5  | grep -q "$STR"; then
    echo "ERROR: Test 9 and 10 not enabled"
    exit 1
fi

# check 01-04
grep "TEST 01-accept-ping PASSING" results
grep "TEST 02-pending-accept PASSING" results
grep "TEST 03-many-pending-accept PASSING" results
grep "TEST 04-ping-echo PASSING" results