#!/bin/bash

PORT=$@

# Connect to server and never send any input
nc 127.0.0.1 $PORT &
echo -n ""

sleep 0.1

REQUEST=$'GET /ping HTTP/1.1\r\n\r\n'

# While that connection is pending, attempt another connection
# This second connection should work
echo -n "$REQUEST" | timeout 2 nc 127.0.0.1 $PORT

# Kill the background nc process
killall nc &> /dev/null
echo -n ""
