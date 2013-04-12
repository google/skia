#!/bin/bash

# Self-tests for gm, based on tools/tests/run.sh
#
# These tests are run by the Skia_PerCommit_House_Keeping bot at every commit,
# so make sure that they still pass when you make changes to gm!
#
# To generate new baselines when gm behavior changes, run gm/tests/rebaseline.sh
#
# TODO: because this is written as a shell script (instead of, say, Python)
# it only runs on Linux and Mac.
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
CONFIGS="--config 8888 565"

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
  COMMAND="$GM_BINARY $GM_ARGS --writeJsonSummaryPath $JSON_SUMMARY_FILE"
  echo "$COMMAND" >$ACTUAL_OUTPUT_DIR/command_line
  $COMMAND >$ACTUAL_OUTPUT_DIR/stdout 2>$ACTUAL_OUTPUT_DIR/stderr
  echo $? >$ACTUAL_OUTPUT_DIR/return_value

  # Only compare selected lines in the stdout, to ignore any spurious lines
  # as noted in http://code.google.com/p/skia/issues/detail?id=1068 .
  #
  # TODO(epoger): This is still hacky... we need to rewrite this script in
  # Python soon, and make stuff like this more maintainable.
  grep ^GM: $ACTUAL_OUTPUT_DIR/stdout >$ACTUAL_OUTPUT_DIR/stdout-tmp
  mv $ACTUAL_OUTPUT_DIR/stdout-tmp $ACTUAL_OUTPUT_DIR/stdout
  grep ^GM: $ACTUAL_OUTPUT_DIR/stderr >$ACTUAL_OUTPUT_DIR/stderr-tmp
  mv $ACTUAL_OUTPUT_DIR/stderr-tmp $ACTUAL_OUTPUT_DIR/stderr

  compare_directories $EXPECTED_OUTPUT_DIR $ACTUAL_OUTPUT_DIR
}

# Create input dir (at path $1) with expectations (both image and json)
# that gm will match or mismatch as appropriate.
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
  IMAGES_DIR=$INPUTS_DIR/images
  JSON_DIR=$INPUTS_DIR/json
  mkdir -p $IMAGES_DIR $JSON_DIR

  mkdir -p $IMAGES_DIR/identical-bytes
  # Run GM to write out the images actually generated.
  $GM_BINARY --hierarchy --match selftest1 $CONFIGS \
    -w $IMAGES_DIR/identical-bytes
  # Run GM again to read in those images and write them out as a JSON summary.
  $GM_BINARY --hierarchy --match selftest1 $CONFIGS \
    -r $IMAGES_DIR/identical-bytes \
    --writeJsonSummaryPath $JSON_DIR/identical-bytes.json

  mkdir -p $IMAGES_DIR/identical-pixels
  $GM_BINARY --hierarchy --match selftest1 $CONFIGS \
    -w $IMAGES_DIR/identical-pixels
  echo "more bytes that do not change the image pixels" \
    >> $IMAGES_DIR/identical-pixels/8888/selftest1.png
  echo "more bytes that do not change the image pixels" \
    >> $IMAGES_DIR/identical-pixels/565/selftest1.png
  $GM_BINARY --hierarchy --match selftest1 $CONFIGS \
    -r $IMAGES_DIR/identical-pixels \
    --writeJsonSummaryPath $JSON_DIR/identical-pixels.json

  mkdir -p $IMAGES_DIR/different-pixels
  $GM_BINARY --hierarchy --match selftest2 $CONFIGS \
    -w $IMAGES_DIR/different-pixels
  mv $IMAGES_DIR/different-pixels/8888/selftest2.png \
    $IMAGES_DIR/different-pixels/8888/selftest1.png
  mv $IMAGES_DIR/different-pixels/565/selftest2.png \
    $IMAGES_DIR/different-pixels/565/selftest1.png
  $GM_BINARY --hierarchy --match selftest1 $CONFIGS \
    -r $IMAGES_DIR/different-pixels \
    --writeJsonSummaryPath $JSON_DIR/different-pixels.json

  mkdir -p $IMAGES_DIR/empty-dir
}

GM_TESTDIR=gm/tests
GM_INPUTS=$GM_TESTDIR/inputs
GM_OUTPUTS=$GM_TESTDIR/outputs
GM_TEMPFILES=$GM_TESTDIR/tempfiles

create_inputs_dir $GM_INPUTS

# Compare generated image against an input image file with identical bytes.
gm_test "--verbose --hierarchy --match selftest1 $CONFIGS -r $GM_INPUTS/images/identical-bytes" "$GM_OUTPUTS/compared-against-identical-bytes-images"
gm_test "--verbose --hierarchy --match selftest1 $CONFIGS -r $GM_INPUTS/json/identical-bytes.json" "$GM_OUTPUTS/compared-against-identical-bytes-json"

# Compare generated image against an input image file with identical pixels but different PNG encoding.
gm_test "--verbose --hierarchy --match selftest1 $CONFIGS -r $GM_INPUTS/images/identical-pixels" "$GM_OUTPUTS/compared-against-identical-pixels-images"
gm_test "--verbose --hierarchy --match selftest1 $CONFIGS -r $GM_INPUTS/json/identical-pixels.json" "$GM_OUTPUTS/compared-against-identical-pixels-json"

# Compare generated image against an input image file with different pixels.
gm_test "--verbose --hierarchy --match selftest1 $CONFIGS -r $GM_INPUTS/images/different-pixels" "$GM_OUTPUTS/compared-against-different-pixels-images"
gm_test "--verbose --hierarchy --match selftest1 $CONFIGS -r $GM_INPUTS/json/different-pixels.json" "$GM_OUTPUTS/compared-against-different-pixels-json"

# Compare generated image against an empty "expected image" dir.
gm_test "--verbose --hierarchy --match selftest1 $CONFIGS -r $GM_INPUTS/images/empty-dir" "$GM_OUTPUTS/compared-against-empty-dir"

# Compare generated image against an empty "expected image" dir, but NOT in verbose mode.
gm_test "--hierarchy --match selftest1 $CONFIGS -r $GM_INPUTS/images/empty-dir" "$GM_OUTPUTS/nonverbose"

# If run without "-r", the JSON's "actual-results" section should contain
# actual checksums marked as "failure-ignored", but the "expected-results"
# section should be empty.
gm_test "--verbose --hierarchy --match selftest1 $CONFIGS" "$GM_OUTPUTS/no-readpath"

# Test what happens if a subset of the renderModes fail (e.g. pipe)
gm_test "--simulatePipePlaybackFailure --verbose --hierarchy --match selftest1 $CONFIGS -r $GM_INPUTS/json/identical-pixels.json" "$GM_OUTPUTS/pipe-playback-failure"

# Confirm that IntentionallySkipped tests are recorded as such.
gm_test "--verbose --hierarchy --match selftest1 selftest2 $CONFIGS" "$GM_OUTPUTS/intentionally-skipped-tests"

# Ignore some error types (including ExpectationsMismatch)
gm_test "--ignoreErrorTypes ExpectationsMismatch NoGpuContext --verbose --hierarchy --match selftest1 $CONFIGS -r $GM_INPUTS/json/different-pixels.json" "$GM_OUTPUTS/ignore-expectations-mismatch"

echo "All tests passed."
