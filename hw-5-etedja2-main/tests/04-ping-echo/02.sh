#!/bin/bash

PORT=$@

REQUEST=$'GET /echo HTTP/1.1\r\nHeader: Value\r\n\r\n'

echo -n "$REQUEST" | nc 127.0.0.1 $PORT
