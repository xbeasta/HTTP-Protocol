#!/bin/bash

PORT=$@

# Connect to server and never send any input x99
for run in {1..99}; do
    nc 127.0.0.1 $PORT &> /dev/null &
done

sleep 1

# Print pending connections (should be 99)
ps | grep nc | wc -l

REQUEST=$'GET /ping HTTP/1.1\r\n\r\n'

# While all those connections are pending, attempt another connection
# This connection should work, there should be one free thread
echo -n "$REQUEST" | timeout 2 nc 127.0.0.1 $PORT

# Kill all background nc processes
killall nc &> /dev/null
echo -n ""
