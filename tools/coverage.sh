#!/bin/sh

# Run from Skia trunk something like this:
#   $ tools/coverage.sh tests
# or
#   $ tools/coverage.sh gm

set -x
set -e

COMMAND=$@
GCOV=$(realpath tools/gcov_shim)

QUIET=-q

# Build all of Skia.
./gyp_skia
ninja -C out/Coverage

# Generate a zero-baseline so files not covered by $COMMAND will still show up in the report.
# This reads the .gcno files that are created at compile time.
lcov $QUIET --gcov-tool=$GCOV -c -b out/Coverage -d out/Coverage -o /tmp/baseline -i

# Running the binary generates the real coverage information, the .gcda files.
out/Coverage/$COMMAND
lcov $QUIET --gcov-tool=$GCOV -c -b out/Coverage -d out/Coverage -o /tmp/coverage

lcov $QUIET -a /tmp/baseline -a /tmp/coverage -o /tmp/merged

genhtml $QUIET /tmp/merged --legend -o out/Coverage/report
xdg-open out/Coverage/report/index.html
