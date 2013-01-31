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

OUTPUT_ACTUAL_SUBDIR=output-actual
OUTPUT_EXPECTED_SUBDIR=output-expected
CONFIGS="--config 8888 --config 565"

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
# - writing stdout into $2/$OUTPUT_ACTUAL_SUBDIR/stdout
# - writing json summary into $2/$OUTPUT_ACTUAL_SUBDIR/json-summary.txt
# - writing return value into $2/$OUTPUT_ACTUAL_SUBDIR/return_value
# Then compare all of those against $2/$OUTPUT_EXPECTED_SUBDIR .
function gm_test {
  if [ $# != 2 ]; then
    echo "gm_test requires exactly 2 parameters, got $#"
    exit 1
  fi
  GM_ARGS="$1"
  ACTUAL_OUTPUT_DIR="$2/$OUTPUT_ACTUAL_SUBDIR"
  EXPECTED_OUTPUT_DIR="$2/$OUTPUT_EXPECTED_SUBDIR"
  JSON_SUMMARY_FILE="$ACTUAL_OUTPUT_DIR/json-summary.txt"

  rm -rf $ACTUAL_OUTPUT_DIR
  mkdir -p $ACTUAL_OUTPUT_DIR
  COMMAND="$GM_BINARY $GM_ARGS --writeJsonSummary $JSON_SUMMARY_FILE"
  echo "$COMMAND" >$ACTUAL_OUTPUT_DIR/command_line
  $COMMAND &>$ACTUAL_OUTPUT_DIR/stdout
  echo $? >$ACTUAL_OUTPUT_DIR/return_value

  # Only compare selected lines in the stdout, to ignore any spurious lines
  # as noted in http://code.google.com/p/skia/issues/detail?id=1068 .
  #
  # TODO(epoger): This is still hacky... we need to rewrite this script in
  # Python soon, and make stuff like this more maintainable.
  grep --regexp=^reading --regexp=^writing --regexp=^drawing \
    --regexp=^FAILED --regexp=^Ran $ACTUAL_OUTPUT_DIR/stdout \
    >$ACTUAL_OUTPUT_DIR/stdout-tmp
  mv $ACTUAL_OUTPUT_DIR/stdout-tmp $ACTUAL_OUTPUT_DIR/stdout

  # Replace particular checksums in json summary with a placeholder, so
  # we don't need to rebaseline these json files when our drawing routines
  # change.
  sed -e 's/"checksum" : [0-9]*/"checksum" : FAKE/g' \
    --in-place $JSON_SUMMARY_FILE
  sed -e 's/"checksums" : \[ [0-9]* \]/"checksums" : [ FAKE ]/g' \
    --in-place $JSON_SUMMARY_FILE

  compare_directories $EXPECTED_OUTPUT_DIR $ACTUAL_OUTPUT_DIR
}

# Create input dir (at path $1) with images that match or mismatch
# as appropriate.
#
# We used to check these files into SVN, but then we needed to rebasline them
# when our drawing changed at all... so, as proposed in
# http://code.google.com/p/skia/issues/detail?id=1068 , we generate them
# new each time.
function create_inputs_dir {
  if [ $# != 1 ]; then
    echo "create_inputs_dir requires exactly 1 parameter, got $#"
    exit 1
  fi
  INPUTS_DIR="$1"
  mkdir -p $INPUTS_DIR

  mkdir -p $INPUTS_DIR/identical-bytes
  $GM_BINARY --hierarchy --match dashing2 $CONFIGS \
    -w $INPUTS_DIR/identical-bytes

  mkdir -p $INPUTS_DIR/identical-pixels
  $GM_BINARY --hierarchy --match dashing2 $CONFIGS \
    -w $INPUTS_DIR/identical-pixels
  echo "more bytes that do not change the image pixels" \
    >> $INPUTS_DIR/identical-pixels/8888/dashing2.png
  echo "more bytes that do not change the image pixels" \
    >> $INPUTS_DIR/identical-pixels/565/dashing2.png

  mkdir -p $INPUTS_DIR/different-pixels
  $GM_BINARY --hierarchy --match dashing3 $CONFIGS \
    -w $INPUTS_DIR/different-pixels
  mv $INPUTS_DIR/different-pixels/8888/dashing3.png \
    $INPUTS_DIR/different-pixels/8888/dashing2.png
  mv $INPUTS_DIR/different-pixels/565/dashing3.png \
    $INPUTS_DIR/different-pixels/565/dashing2.png

  mkdir -p $INPUTS_DIR/empty-dir
}

GM_TESTDIR=gm/tests
GM_INPUTS=$GM_TESTDIR/inputs
GM_OUTPUTS=$GM_TESTDIR/outputs
GM_TEMPFILES=$GM_TESTDIR/tempfiles

create_inputs_dir $GM_INPUTS

# Compare generated image against an input image file with identical bytes.
gm_test "--hierarchy --match dashing2 $CONFIGS -r $GM_INPUTS/identical-bytes" "$GM_OUTPUTS/compared-against-identical-bytes"

# Compare generated image against an input image file with identical pixels but different PNG encoding.
gm_test "--hierarchy --match dashing2 $CONFIGS -r $GM_INPUTS/identical-pixels" "$GM_OUTPUTS/compared-against-identical-pixels"

# Compare generated image against an input image file with different pixels.
gm_test "--hierarchy --match dashing2 $CONFIGS -r $GM_INPUTS/different-pixels" "$GM_OUTPUTS/compared-against-different-pixels"

# Compare generated image against an empty "expected image" dir.
gm_test "--hierarchy --match dashing2 $CONFIGS -r $GM_INPUTS/empty-dir" "$GM_OUTPUTS/compared-against-empty-dir"

# If run without "-r", the JSON's "actual-results" section should contain
# actual checksums marked as "failure-ignored", but the "expected-results"
# section should be empty.
gm_test "--hierarchy --match dashing2 $CONFIGS" "$GM_OUTPUTS/no-readpath"

# Run a test which generates partially transparent images, write out those
# images, and read them back in.
#
# This test would have caught
# http://code.google.com/p/skia/issues/detail?id=1079 ('gm generating
# spurious pixel_error messages as of r7258').
IMAGEDIR=$GM_TEMPFILES/aaclip-images
rm -rf $IMAGEDIR
mkdir -p $IMAGEDIR
gm_test "--match simpleaaclip_path $CONFIGS -w $IMAGEDIR" "$GM_OUTPUTS/aaclip-write"
gm_test "--match simpleaaclip_path $CONFIGS -r $IMAGEDIR" "$GM_OUTPUTS/aaclip-readback"

echo "All tests passed."
