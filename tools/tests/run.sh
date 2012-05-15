#!/bin/bash

# Tests for our tools.
# TODO: for now, it only tests skdiff
# TODO: for now, assumes that it is being run from .../trunk

# TODO: make it look in Release and/or Debug
SKDIFF_BINARY=out/Debug/skdiff

function compare_directories {
  diff --exclude=.* $1 $2
  if [ $? != 0 ]; then
    echo "failed in: compare_directories $1 $2"
    exit 1
  fi
}

# Test skdiff...
#
SKDIFF_TESTDIR=tools/tests/skdiff
SKDIFF_OUTPUT_DIR=$SKDIFF_TESTDIR/output-actual
mkdir -p $SKDIFF_OUTPUT_DIR
$SKDIFF_BINARY $SKDIFF_TESTDIR/baseDir $SKDIFF_TESTDIR/comparisonDir \
  $SKDIFF_OUTPUT_DIR &>$SKDIFF_OUTPUT_DIR/stdout
compare_directories $SKDIFF_TESTDIR/output-expected $SKDIFF_OUTPUT_DIR

echo "All tests passed."
