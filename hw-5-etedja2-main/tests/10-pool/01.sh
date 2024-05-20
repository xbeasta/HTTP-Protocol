#!/bin/bash

./hw5
PORT=$@

telnet 127.0.0.1 $PORT < /dev/null &> /dev/null
echo -n ""
