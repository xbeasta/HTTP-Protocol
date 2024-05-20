#!/bin/bash

STATS=$'GET /stats HTTP/1.1\r\n\r\n'

PORT=$@

PING_REQ=$'GET /ping HTTP/1.1\r\n\r\n'
PING_RESP=$'HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\npong'

send_ping() {

    echo -n "$PING_RESP" &> /dev/null &
}

ECHO_REQ=$'GET /echo HTTP/1.1\r
Host: localhost:9999\r
Connection: keep-alive\r
sec-ch-ua: "Not?A_Brand";v="8", "Chromium";v="108", "Google Chrome";v="108"\r
sec-ch-ua-mobile: ?0\r
sec-ch-ua-platform: "Linux"\r
DNT: 1\r
Upgrade-Insecure-Requests: 1\r
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.0.0 Safari/537.36\r
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r
Sec-Fetch-Site: none\r
Sec-Fetch-Mode: navigate\r
Sec-Fetch-User: ?1\r
Sec-Fetch-Dest: document\r
Accept-Encoding: gzip, deflate, br\r
Accept-Language: en-US,en;q=0.9,pt;q=0.8\r
Cookie: _ga=GA1.1.149590090.1676066969\r\n\r\n'

send_echo() {

    echo -n "$ECHO_REQ" | nc 127.0.0.1 $PORT &> /dev/null &

}

RANDOM_FILE_DATA=`dd if=/dev/urandom count=1000 bs=4096 2> /dev/null | base64`
echo -n "$RANDOM_DATA" > file

send_file() {
    curl -s http://127.0.0.1:$PORT/file &> /dev/null
}


send_error() {
    curl -s http://127.0.0.1:$PORT/f &> /dev/null &
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

send_write

wait


for j in $(seq 100); do
    for i in $(seq 8); do
        send_echo
        send_ping
        send_file
        send_error
        send_write
        send_read
    done

    wait

    echo -n "$STATS" | nc 127.0.0.1 $PORT
    echo ""
done

rm expected_* &> /dev/null
rm file_* &> /dev/null

echo -n "$STATS" | nc 127.0.0.1 $PORT
echo ""