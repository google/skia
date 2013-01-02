#!/bin/bash

# Tests for our tools.
#
# TODO: for now, it only tests skdiff
#
# TODO: currently, this only passes on Linux (which is the platform that
# the housekeeper bot runs on, e.g.
# http://70.32.156.51:10117/builders/Skia_PerCommit_House_Keeping/builds/1415/steps/RunToolSelfTests/logs/stdio )
# See https://code.google.com/p/skia/issues/detail?id=677
# ('make tools/tests/run.sh work cross-platform')
# Ideally, these tests should pass on all development platforms...
# otherwise, how can developers be expected to test them before committing a
# change?

# cd into .../trunk so all the paths will work
cd $(dirname $0)/../..

# TODO: make it look in Release and/or Debug
SKDIFF_BINARY=out/Debug/skdiff

# Compare contents of all files within directories $1 and $2,
# EXCEPT for any dotfiles.
# If there are any differences, a description is written to stdout and
# we exit with a nonzero return value.
# Otherwise, we write nothing to stdout and return.
function compare_directories {
  if [ $# != 2 ]; then
    echo "compare_directories requires exactly 2 parameters, got $#"
    exit 1
  fi
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
  if [ $# != 2 ]; then
    echo "skdiff_test requires exactly 2 parameters, got $#"
    exit 1
  fi
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

# Run skdiff over a variety of file pair types: identical bits, identical pixels, missing from
# baseDir, etc.
skdiff_test "$SKDIFF_TESTDIR/baseDir $SKDIFF_TESTDIR/comparisonDir" "$SKDIFF_TESTDIR/test1"

# Run skdiff over the same set of files, but with arguments as used by our buildbots:
# - return the number of mismatching file pairs (but ignore any files missing from either
#   baseDir or comparisonDir)
# - list filenames with each result type to stdout
# - don't generate HTML output files
skdiff_test "--failonresult DifferentPixels --failonresult DifferentSizes --failonresult Unknown --failonstatus CouldNotDecode,CouldNotRead any --failonstatus any CouldNotDecode,CouldNotRead --listfilenames --nodiffs $SKDIFF_TESTDIR/baseDir $SKDIFF_TESTDIR/comparisonDir" "$SKDIFF_TESTDIR/test2"

# Run skdiff over just the files that have identical bits.
skdiff_test "--nodiffs --match identical-bits $SKDIFF_TESTDIR/baseDir $SKDIFF_TESTDIR/comparisonDir" "$SKDIFF_TESTDIR/identical-bits"

# Run skdiff over just the files that have identical bits or identical pixels.
skdiff_test "--nodiffs --match identical-bits --match identical-pixels $SKDIFF_TESTDIR/baseDir $SKDIFF_TESTDIR/comparisonDir" "$SKDIFF_TESTDIR/identical-bits-or-pixels"

echo "All tests passed."
