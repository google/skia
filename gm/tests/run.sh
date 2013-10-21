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

ENCOUNTERED_ANY_ERRORS=0

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
    ENCOUNTERED_ANY_ERRORS=1
  fi
}

# Run a command, and validate that it succeeds (returns 0).
function assert_passes {
  COMMAND="$1"
  OUTPUT=$($COMMAND 2>&1)
  if [ $? != 0 ]; then
    echo "This command was supposed to pass, but failed: [$COMMAND]"
    echo $OUTPUT
    ENCOUNTERED_ANY_ERRORS=1
  fi
}

# Run a command, and validate that it fails (returns nonzero).
function assert_fails {
  COMMAND="$1"
  OUTPUT=$($COMMAND 2>&1)
  if [ $? == 0 ]; then
    echo "This command was supposed to fail, but passed: [$COMMAND]"
    echo $OUTPUT
    ENCOUNTERED_ANY_ERRORS=1
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

  COMMAND="$GM_BINARY $GM_ARGS --writeJsonSummaryPath $JSON_SUMMARY_FILE --writePath $ACTUAL_OUTPUT_DIR/writePath --mismatchPath $ACTUAL_OUTPUT_DIR/mismatchPath --missingExpectationsPath $ACTUAL_OUTPUT_DIR/missingExpectationsPath"

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

  # Replace image file contents with just the filename, for two reasons:
  # 1. Image file encoding may vary by platform
  # 2. https://code.google.com/p/chromium/issues/detail?id=169600
  #    ('gcl/upload.py fail to upload binary files to rietveld')
  for IMAGEFILE in $(find $ACTUAL_OUTPUT_DIR -name *.png); do
    echo "[contents of $IMAGEFILE]" >$IMAGEFILE
  done
  for IMAGEFILE in $(find $ACTUAL_OUTPUT_DIR -name *.pdf); do
    echo "[contents of $IMAGEFILE]" >$IMAGEFILE
  done

  # Add a file to any empty subdirectories.
  for DIR in $(find $ACTUAL_OUTPUT_DIR -mindepth 1 -type d); do
    echo "Created additional file to make sure directory isn't empty, because self-test cannot handle empty directories." >$DIR/bogusfile
  done

  compare_directories $EXPECTED_OUTPUT_DIR $ACTUAL_OUTPUT_DIR
}

# Create input dir (at path $1) with expectations (both image and json)
# that gm will match or mismatch as appropriate.
#
# We used to check these files into SVN, but then we needed to rebaseline them
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

  THIS_IMAGE_DIR=$IMAGES_DIR/identical-bytes
  mkdir -p $THIS_IMAGE_DIR
  # Run GM to write out the images actually generated.
  $GM_BINARY --hierarchy --match selftest1 $CONFIGS -w $THIS_IMAGE_DIR
  # Run GM again to read in those images and write them out as a JSON summary.
  $GM_BINARY --hierarchy --match selftest1 $CONFIGS -r $THIS_IMAGE_DIR \
    --writeJsonSummaryPath $JSON_DIR/identical-bytes.json

  THIS_IMAGE_DIR=$IMAGES_DIR/identical-pixels
  mkdir -p $THIS_IMAGE_DIR
  $GM_BINARY --hierarchy --match selftest1 $CONFIGS -w $THIS_IMAGE_DIR
  echo "more bytes that do not change the image pixels" \
    >> $THIS_IMAGE_DIR/8888/selftest1.png
  echo "more bytes that do not change the image pixels" \
    >> $THIS_IMAGE_DIR/565/selftest1.png
  $GM_BINARY --hierarchy --match selftest1 $CONFIGS -r $THIS_IMAGE_DIR \
    --writeJsonSummaryPath $JSON_DIR/identical-pixels.json

  THIS_IMAGE_DIR=$IMAGES_DIR/different-pixels
  mkdir -p $THIS_IMAGE_DIR
  $GM_BINARY --hierarchy --match selftest2 $CONFIGS -w $THIS_IMAGE_DIR
  mv $THIS_IMAGE_DIR/8888/selftest2.png $THIS_IMAGE_DIR/8888/selftest1.png
  mv $THIS_IMAGE_DIR/565/selftest2.png  $THIS_IMAGE_DIR/565/selftest1.png
  $GM_BINARY --hierarchy --match selftest1 $CONFIGS -r $THIS_IMAGE_DIR \
    --writeJsonSummaryPath $JSON_DIR/different-pixels.json

  # Create another JSON expectations file which is identical to
  # different-pixels.json, but in which the *first* ignore-failure is changed
  # from false to true.
  OLD='"ignore-failure" : false'
  NEW='"ignore-failure" : true'
  sed -e "0,/$OLD/{s/$OLD/$NEW/}" $JSON_DIR/different-pixels.json \
      >$JSON_DIR/different-pixels-ignore-some-failures.json

  THIS_IMAGE_DIR=$IMAGES_DIR/different-pixels-no-hierarchy
  mkdir -p $THIS_IMAGE_DIR
  $GM_BINARY --match selftest2 $CONFIGS -w $THIS_IMAGE_DIR
  mv $THIS_IMAGE_DIR/selftest2_8888.png $THIS_IMAGE_DIR/selftest1_8888.png
  mv $THIS_IMAGE_DIR/selftest2_565.png  $THIS_IMAGE_DIR/selftest1_565.png
  $GM_BINARY --match selftest1 $CONFIGS -r $THIS_IMAGE_DIR \
    --writeJsonSummaryPath $JSON_DIR/different-pixels-no-hierarchy.json

  mkdir -p $IMAGES_DIR/empty-dir

  echo "# Comment line" >$GM_IGNORE_FAILURES_FILE
  echo "" >>$GM_IGNORE_FAILURES_FILE
  echo "# ignore any test runs whose filename contains '8888/selfte'" >>$GM_IGNORE_FAILURES_FILE
  echo "#   (in other words, config is 8888 and test name starts with 'selfte')" >>$GM_IGNORE_FAILURES_FILE
  echo "8888/selfte" >>$GM_IGNORE_FAILURES_FILE
}

GM_TESTDIR=gm/tests
GM_INPUTS=$GM_TESTDIR/inputs
GM_OUTPUTS=$GM_TESTDIR/outputs
GM_TEMPFILES=$GM_TESTDIR/tempfiles
GM_IGNORE_FAILURES_FILE=$GM_INPUTS/ignored-tests.txt

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

# Exercise --ignoreFailuresFile flag.
gm_test "--verbose --hierarchy --match selftest1 --ignoreFailuresFile $GM_IGNORE_FAILURES_FILE $CONFIGS -r $GM_INPUTS/json/different-pixels.json" "$GM_OUTPUTS/ignoring-one-test"

# Compare different pixels, but with a SUBSET of the expectations marked as
# ignore-failure.
gm_test "--verbose --hierarchy --match selftest1 $CONFIGS -r $GM_INPUTS/json/different-pixels-ignore-some-failures.json" "$GM_OUTPUTS/ignoring-some-failures"

# Compare generated image against an empty "expected image" dir.
# Even the tests that have been marked as ignore-failure should show up as
# no-comparison.
gm_test "--verbose --hierarchy --match selftest1 --ignoreFailuresFile $GM_IGNORE_FAILURES_FILE $CONFIGS -r $GM_INPUTS/images/empty-dir" "$GM_OUTPUTS/compared-against-empty-dir"

# Compare generated image against a nonexistent "expected image" dir.
gm_test "--verbose --hierarchy --match selftest1 $CONFIGS -r ../path/to/nowhere" "$GM_OUTPUTS/compared-against-nonexistent-dir"

# Compare generated image against an empty "expected image" dir, but NOT in verbose mode.
gm_test "--hierarchy --match selftest1 $CONFIGS -r $GM_INPUTS/images/empty-dir" "$GM_OUTPUTS/nonverbose"

# Add pdf to the list of configs.
gm_test "--verbose --hierarchy --match selftest1 $CONFIGS pdf -r $GM_INPUTS/json/identical-bytes.json" "$GM_OUTPUTS/add-config-pdf"

# Test what happens if run without -r (no expected-results.json to compare
# against).
gm_test "--verbose --hierarchy --match selftest1 $CONFIGS" "$GM_OUTPUTS/no-readpath"

# Test what happens if a subset of the renderModes fail (e.g. pipe)
gm_test "--pipe --simulatePipePlaybackFailure --verbose --hierarchy --match selftest1 $CONFIGS -r $GM_INPUTS/json/identical-pixels.json" "$GM_OUTPUTS/pipe-playback-failure"

# Confirm that IntentionallySkipped tests are recorded as such.
gm_test "--verbose --hierarchy --match selftest1 selftest2 $CONFIGS" "$GM_OUTPUTS/intentionally-skipped-tests"

# Ignore some error types (including ExpectationsMismatch)
gm_test "--ignoreErrorTypes ExpectationsMismatch NoGpuContext --verbose --hierarchy --match selftest1 $CONFIGS -r $GM_INPUTS/json/different-pixels.json" "$GM_OUTPUTS/ignore-expectations-mismatch"

# Test non-hierarchical mode.
gm_test "--verbose --match selftest1 $CONFIGS -r $GM_INPUTS/json/different-pixels-no-hierarchy.json" "$GM_OUTPUTS/no-hierarchy"

# Try writing out actual images using checksum-based filenames, like we do when
# uploading to Google Storage.
gm_test "--verbose --writeChecksumBasedFilenames --match selftest1 $CONFIGS -r $GM_INPUTS/json/different-pixels-no-hierarchy.json" "$GM_OUTPUTS/checksum-based-filenames"

# Exercise display_json_results.py
PASSING_CASES="compared-against-identical-bytes-json compared-against-identical-pixels-json"
FAILING_CASES="compared-against-different-pixels-json"
for CASE in $PASSING_CASES; do
  assert_passes "python gm/display_json_results.py $GM_OUTPUTS/$CASE/$OUTPUT_EXPECTED_SUBDIR/json-summary.txt"
done
for CASE in $FAILING_CASES; do
  assert_fails "python gm/display_json_results.py $GM_OUTPUTS/$CASE/$OUTPUT_EXPECTED_SUBDIR/json-summary.txt"
done

if [ $ENCOUNTERED_ANY_ERRORS == 0 ]; then
  echo "All tests passed."
  exit 0
else
  exit 1
fi
