#!/bin/bash

PORT=$@

REQUEST=$'GET /ping HTTP/1.1\r\n\r\n'

# Connect to server and never send any input x99, leaving only one thread left
for run in {1..99}; do
    nc 127.0.0.1 $PORT &
done

sleep 1

# Print pending connections (should be 99)
ps | grep nc | wc -l

# While all those connections are pending, attempt another connections
# These connection should work, there should be one free thread
for run in {1..50}; do
    echo -n "$REQUEST" | timeout 2 nc 127.0.0.1 $PORT
done

# Kill all background nc processes
killall nc &> /dev/null
echo -n ""