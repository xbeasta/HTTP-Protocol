#!/bin/bash

PORT=$@

send_ping() {
    PING_REQ=$'GET /ping HTTP/1.1\r\n\r\n'
    PING_RESP=$'HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\npong'

    echo -n "$PING_RESP" > expected_ping_$1
    echo -n "$PING_REQ" | nc 127.0.0.1 $PORT | diff expected_ping_$1 - &
}

send_echo() {
    EOL=$'\r\n'
    REQUEST="GET /echo HTTP/1.1"
    EXPECTED=""

    VARS=$(($RANDOM % 20))

    NAME_LEN=$((1 + $RANDOM % 20))
    NAME=`dd if=/dev/urandom count=1 bs=20 2> /dev/null | base64 | head -c $NAME_LEN | sed s/://g`
    CONTENT_LEN=$((1 + $RANDOM % 20))
    CONTENT=`dd if=/dev/urandom count=1 bs=20 2> /dev/null | base64 | head -c $CONTENT_LEN | sed s/://g`

    VAR="$NAME: $CONTENT"
    REQUEST="$REQUEST$EOL$VAR"
    EXPECTED="$VAR"

    for i in $(seq $VARS); do
        NAME_LEN=$((1 + $RANDOM % 20))
        NAME=`dd if=/dev/urandom count=1 bs=20 2> /dev/null | base64 | head -c $NAME_LEN | sed s/://g`
        CONTENT_LEN=$((1 + $RANDOM % 20))
        CONTENT=`dd if=/dev/urandom count=1 bs=20 2> /dev/null | base64 | head -c $CONTENT_LEN | sed s/://g`

        VAR="$NAME: $CONTENT"
        REQUEST="$REQUEST$EOL$VAR"
        EXPECTED="$EXPECTED$EOL$VAR"
    done

    REQUEST="$REQUEST$EOL$EOL"

    echo -n "$EXPECTED" > expected_echo_$1
    echo -n "$REQUEST" | nc 127.0.0.1 $PORT | tail -n +4 | diff expected_echo_$1 - &

}

RANDOM_DATA_POST=`dd if=/dev/urandom count=1 bs=1000 2> /dev/null | base64 | head -c 1000`
echo -n "$RANDOM_DATA_POST" > expected_post

send_write() {
    EOL=$'\r'
    REQUEST="POST /write HTTP/1.1$EOL
Content-Length: 1000$EOL
$EOL
$RANDOM_DATA_POST$EOL$EOL$EOL$EOL"

    echo -n "$REQUEST" | nc 127.0.0.1 $PORT | tail -n +4 | diff expected_post - &
}

send_read() {
    REQUEST=$'GET /read HTTP/1.1\r\n\r\n'

    echo -n "$REQUEST" | nc 127.0.0.1 $PORT | tail -n +4 | diff expected_post - &
}

RANDOM_FILE_DATA=`dd if=/dev/urandom count=1000 bs=4096 2> /dev/null | base64`
echo -n "$RANDOM_DATA" > file

md5sum file | head -c 32 > expected_file

send_file() {
    curl -s http://127.0.0.1:$PORT/file | md5sum - | head -c 32 | diff expected_file - &
}

send_write

wait

for i in $(seq 10); do
    send_echo $i
    send_write
    send_ping $i
    send_read
    send_file
done

wait

rm expected_* &> /dev/null
rm file_* &> /dev/null

echo ""
