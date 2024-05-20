#!/bin/bash
#
rm read expected &> /dev/null

PORT=$@

# Create 5K file
LARGE_RANDOM_DATA=`dd if=/dev/urandom count=10 bs=1M 2> /dev/null | base64 > file`
SMALL_RANDOM_DATA=`dd if=/dev/urandom count=2 bs=1k 2> /dev/null | base64 > file2`

# Start 3 big download, limited to 1M

curl --limit-rate 1M -s http://127.0.0.1:$PORT/file | diff file - &
curl --limit-rate 1M -s http://127.0.0.1:$PORT/file | diff file - &
curl --limit-rate 1M -s http://127.0.0.1:$PORT/file | diff file - &

# There should be one thread available to do more work

sleep 1

# Start many small downloads, as fast as possible
for i in $(seq 50); do
    START=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
    if ! curl -s http://127.0.0.1:$PORT/file2 | diff file2 - ; then
        echo "Downloaded data does not match for file2"
        exit 1
    fi
    END=$(date +%s%N | cut -b1-13 | sed s/N/000/g)
    # Small download should have taken less than 1 second
    if [ $((END-START)) -gt "1000" ]; then
        echo "Should have taken less than 1 sec (1000ms)"
        exit 1
    fi
done

wait

exit 0
