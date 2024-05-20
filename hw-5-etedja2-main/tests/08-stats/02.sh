#!/bin/bash

PORT=$@

PING=$'GET /ping HTTP/1.1\r\n\r\n'
STATS=$'GET /stats HTTP/1.1\r\n\r\n'


for j in $(seq 10); do
    for i in $(seq 100); do
        echo -n "$PING" | nc 127.0.0.1 $PORT &> /dev/null &
    done

    wait

    echo -n "$STATS" | nc 127.0.0.1 $PORT
    echo ""
done
