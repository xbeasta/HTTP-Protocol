#!/bin/bash

PORT=$@

REQUEST=$'GET /ping HTTP/1.1\r\n\r\n'

# Connect to server and never send any input x10
for run in {1..10}; do
    nc 127.0.0.1 $PORT &
done

# Connect to server and send a ton of pings to start with
for run in {1..200}; do
    echo -n "$REQUEST" | nc 127.0.0.1 $PORT
done

echo ""

# Connect to server and never send any input x89
for run in {1..89}; do
    nc 127.0.0.1 $PORT &
done

sleep 1

# Print pending connections (should be 99)
ps | grep nc | wc -l


# While all those connections are pending, attempt another connection
# This connection should work, there should be one free thread
echo -n "$REQUEST" | timeout 2 nc 127.0.0.1 $PORT

# Kill all background nc processes
killall nc &> /dev/null
echo -n ""