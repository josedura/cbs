#!/bin/bash

# Getting coverage on run_server with unit tests is quite difficult since we would need
# to get quite deep into mocking boost
# Instead we do a small integration test.

TMP_FILE=$(mktemp)

# Set up a trap to delete the temporary file on exit
trap "rm -f $TMP_FILE" EXIT

echo Starting server
./build/cbs 127.0.0.1 18080 10 >/dev/null &

sleep 10s


echo "Test listmovies"
curl -i http://localhost:18080/api/listmovies > $TMP_FILE 2>/dev/null
if [ $? -ne 0 ]; then
  echo "listmovies request failed"
  exit 1
fi
if ! grep -q "200 OK" $TMP_FILE; then
  echo "listmovies request is not 200 OK"
  exit 2
fi

echo "Test listtheaters"
curl -i http://localhost:18080/api/listtheaters_1 > $TMP_FILE 2>/dev/null
if [ $? -ne 0 ]; then
  echo "listtheaters request failed"
  exit 3
fi
if ! grep -q "200 OK" $TMP_FILE; then
  echo "listtheaters request is not 200 OK"
  exit 4
fi

killall -SIGTERM cbs
sleep 0.1s
# cbs blocks on accept, so will not process the flag from SIGTERM until
# it gets a request
curl -i http://localhost:18080/api/listmovies > /dev/null 2>&1

wait

echo "All integration scenarios are successful"
