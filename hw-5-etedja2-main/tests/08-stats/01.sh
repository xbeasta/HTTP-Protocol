#!/bin/bash

PORT=$@

REQUEST=$'GET /stats HTTP/1.1\r\n\r\n'
PING=$'GET /ping HTTP/1.1\r\n\r\n'

echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
#curl -s http://127.0.0.1:$PORT/ping &> /dev/null
echo -n "$PING" | nc 127.0.0.1 $PORT &> /dev/null
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
curl -s http://127.0.0.1:$PORT/echo &> /dev/null
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
#curl -s http://127.0.0.1:$PORT/ping &> /dev/null
echo -n "$PING" | nc 127.0.0.1 $PORT &> /dev/null
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
curl -s http://127.0.0.1:$PORT/echo &> /dev/null
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
curl -s http://127.0.0.1:$PORT/echo &> /dev/null
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
curl -s http://127.0.0.1:$PORT/read &> /dev/null
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
curl -s http://127.0.0.1:$PORT/echo &> /dev/null
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
curl -s http://127.0.0.1:$PORT/tests/05-file/threads.txt &> /dev/null
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
curl -s http://127.0.0.1:$PORT/tests/06-files/index.html &> /dev/null
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
curl -s http://127.0.0.1:$PORT/tests/06-error/index.html &> /dev/null
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
curl -s http://127.0.0.1:$PORT/tests/06-error/index.html &> /dev/null
echo -n "$REQUEST" | nc 127.0.0.1 $PORT
echo ""
