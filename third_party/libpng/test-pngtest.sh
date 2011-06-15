#!/bin/sh

echo "Running tests.  For details see pngtest-log.txt"

echo "============ pngtest pngtest.png ==============" > pngtest-log.txt

echo "Running test-pngtest.sh"
./pngtest ${srcdir}/pngtest.png >> pngtest-log.txt 2>&1
