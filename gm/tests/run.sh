#!/bin/bash

# Self-tests for gm, based on tools/tests/run.sh
#
# These tests are run by the Skia_PerCommit_House_Keeping bot at every commit,
# so make sure that they still pass when you make changes to gm!
#
# TODO: currently, this only passes on Linux (which is the platform that
# the housekeeper bot runs on, e.g.
# http://70.32.156.51:10117/builders/Skia_PerCommit_House_Keeping/builds/1417/steps/RunGmSelfTests/logs/stdio )
# See https://code.google.com/p/skia/issues/detail?id=677
# ('make tools/tests/run.sh work cross-platform')
# Ideally, these tests should pass on all development platforms...
# otherwise, how can developers be expected to test them before committing a
# change?

# cd into .../trunk so all the paths will work
cd $(dirname $0)/../..

# TODO(epoger): make it look in Release and/or Debug
GM_BINARY=out/Debug/gm

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
  diff -r --exclude=.* $1 $2
  if [ $? != 0 ]; then
    echo "failed in: compare_directories $1 $2"
    exit 1
  fi
}

# Run gm...
# - with the arguments in $1
# - writing resulting images into $2/output-actual/images
# - writing stdout into $2/output-actual/stdout
# - writing json summary into $2/output-actual/json-summary.txt
# - writing return value into $2/output-actual/return_value
# Then compare all of those against $2/output-expected .
function gm_test {
  if [ $# != 2 ]; then
    echo "gm_test requires exactly 2 parameters, got $#"
    exit 1
  fi
  GM_ARGS="$1"
  ACTUAL_OUTPUT_DIR="$2/output-actual"
  EXPECTED_OUTPUT_DIR="$2/output-expected"

  rm -rf $ACTUAL_OUTPUT_DIR
  mkdir -p $ACTUAL_OUTPUT_DIR
  COMMAND="$GM_BINARY $GM_ARGS --writeJsonSummary $ACTUAL_OUTPUT_DIR/json-summary.txt -w $ACTUAL_OUTPUT_DIR/images"
  echo "$COMMAND" >$ACTUAL_OUTPUT_DIR/command_line
  $COMMAND &>$ACTUAL_OUTPUT_DIR/stdout
  echo $? >$ACTUAL_OUTPUT_DIR/return_value

  compare_directories $EXPECTED_OUTPUT_DIR $ACTUAL_OUTPUT_DIR
}

GM_TESTDIR=gm/tests
GM_INPUTS=$GM_TESTDIR/inputs
GM_OUTPUTS=$GM_TESTDIR/outputs

# Compare generated image against an input image file with identical bytes.
gm_test "--hierarchy --match dashing2 --config 8888 -r $GM_INPUTS/identical-bytes" "$GM_OUTPUTS/compared-against-identical-bytes"

# Compare generated image against an input image file with identical pixels but different PNG encoding.
gm_test "--hierarchy --match dashing2 --config 8888 -r $GM_INPUTS/identical-pixels" "$GM_OUTPUTS/compared-against-identical-pixels"

# Compare generated image against an input image file with different pixels.
gm_test "--hierarchy --match dashing2 --config 8888 -r $GM_INPUTS/different-pixels" "$GM_OUTPUTS/compared-against-different-pixels"

# Compare generated image against an empty "expected image" dir.
gm_test "--hierarchy --match dashing2 --config 8888 -r $GM_INPUTS/empty-dir" "$GM_OUTPUTS/compared-against-empty-dir"

# If run without "-r", the JSON's "actual-results" section should contain
# actual checksums marked as "failure-ignored", but the "expected-results"
# section should be empty.
gm_test "--hierarchy --match dashing2 --config 8888" "$GM_OUTPUTS/no-readpath"

echo "All tests passed."
