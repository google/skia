#!/bin/bash

# Tests for our tools.
# TODO: for now, it only tests skdiff
# TODO: for now, assumes that it is being run from .../trunk

# TODO: make it look in Release and/or Debug
SKDIFF_BINARY=out/Debug/skdiff

# Compare contents of all files within directories $1 and $2,
# EXCEPT for any dotfiles.
# If there are any differences, a description is written to stdout and
# we exit with a nonzero return value.
# Otherwise, we write nothing to stdout and return.
function compare_directories {
  diff --exclude=.* $1 $2
  if [ $? != 0 ]; then
    echo "failed in: compare_directories $1 $2"
    exit 1
  fi
}

# Run skdiff with arguments in $1 (plus implicit final argument causing skdiff
# to write its output, if any, to directory $2/output-actual).
# Then compare its results against those in $2/output-expected.
function skdiff_test {
  SKDIFF_ARGS="$1"
  ACTUAL_OUTPUT_DIR="$2/output-actual"
  EXPECTED_OUTPUT_DIR="$2/output-expected"

  rm -rf $ACTUAL_OUTPUT_DIR
  mkdir -p $ACTUAL_OUTPUT_DIR
  COMMAND="$SKDIFF_BINARY $SKDIFF_ARGS $ACTUAL_OUTPUT_DIR"
  echo "$COMMAND" >$ACTUAL_OUTPUT_DIR/command_line
  $COMMAND &>$ACTUAL_OUTPUT_DIR/stdout
  echo $? >$ACTUAL_OUTPUT_DIR/return_value

  compare_directories $EXPECTED_OUTPUT_DIR $ACTUAL_OUTPUT_DIR
}

SKDIFF_TESTDIR=tools/tests/skdiff

# Run skdiff over a variety of file pair types: identical bits, identical
# pixels, missing from baseDir, etc.
# TODO: In the near future, skdiff will return a nonzero exit code in this case.
skdiff_test "$SKDIFF_TESTDIR/baseDir $SKDIFF_TESTDIR/comparisonDir" "$SKDIFF_TESTDIR/test1"

# Same as above but without generating HTML output files.
# TODO: In the near future, skdiff will return a nonzero exit code in this case.
skdiff_test "--nodiffs $SKDIFF_TESTDIR/baseDir $SKDIFF_TESTDIR/comparisonDir" "$SKDIFF_TESTDIR/test2"

# Run skdiff over just the files that have identical bits, to validate any
# behavior/return value differences in this case.
skdiff_test "--nodiffs --match identical-bits $SKDIFF_TESTDIR/baseDir $SKDIFF_TESTDIR/comparisonDir" "$SKDIFF_TESTDIR/identical-bits"

# Run skdiff over just the files that have identical bits or identical pixels,
# to validate any behavior/return value differences in this case.
skdiff_test "--nodiffs --match identical-bits --match identical-pixels $SKDIFF_TESTDIR/baseDir $SKDIFF_TESTDIR/comparisonDir" "$SKDIFF_TESTDIR/identical-bits-or-pixels"

echo "All tests passed."
