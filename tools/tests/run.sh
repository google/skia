#!/bin/bash

# Tests for our tools.
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

# Suffixes of the raw bench data files we want to process.
BENCHDATA_FILE_SUFFIXES_YES_INDIVIDUAL_TILES=\
"data_skp_scale_1.3061_config_8888_mode_tile_256_256_timeIndividualTiles_bbh_rtree "\
"data_skp_scale_1.3061_config_8888_mode_tile_256_256_timeIndividualTiles"
BENCHDATA_FILE_SUFFIXES_NO_INDIVIDUAL_TILES=\
"data_skp_multi_4_scale_1.3061_config_8888_mode_tile_256_256 "\
"data_skp_scale_1.3061_config_8888_mode_record"

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
  diff --recursive --exclude=.* $1 $2
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

# Download a subset of the raw bench data for platform $1 at revision $2.
# (For the subset, download all files matching any of the suffixes in
# whitespace-separated list $3.)
# If any of those files already exist locally, we assume that they are
# correct and up to date, and we don't download them again.
function download_bench_rawdata {
  if [ $# != 3 ]; then
    echo "download_bench_rawdata requires exactly 3 parameters, got $#"
    exit 1
  fi
  PLATFORM="$1"
  REV="$2"
  FILE_SUFFIXES="$3"

  PLATFORM_DIR="tools/tests/benchalerts/$PLATFORM"
  RAW_BENCH_DATA_DIR="$PLATFORM_DIR/raw-bench-data"
  mkdir -p $RAW_BENCH_DATA_DIR

  for FILE_SUFFIX in $FILE_SUFFIXES; do
    FILE=bench_${REV}_${FILE_SUFFIX}
    DESTFILE=$RAW_BENCH_DATA_DIR/$FILE
    if [ ! -f $DESTFILE ];
    then
      URL=http://chromium-skia-gm.commondatastorage.googleapis.com/perfdata/${PLATFORM}/${FILE}
      echo Downloading $URL ...
      curl $URL --output $DESTFILE
    fi
  done
}

# Run check_bench_regressions.py across the data from platform $1,
# writing its output to output-actual and comparing those results against
# output-expected.
function benchalert_test {
  if [ $# != 2 ]; then
    echo "benchalert_test requires exactly 2 parameter, got $#"
    exit 1
  fi
  PLATFORM="$1"
  REVISION="$2"

  PLATFORM_DIR="tools/tests/benchalerts/$PLATFORM"
  RAW_BENCH_DATA_DIR="$PLATFORM_DIR/raw-bench-data"
  ACTUAL_OUTPUT_DIR="$PLATFORM_DIR/output-actual"
  EXPECTED_OUTPUT_DIR="$PLATFORM_DIR/output-expected"

  # Run check_bench_regressions.py .
  rm -rf $ACTUAL_OUTPUT_DIR
  mkdir -p $ACTUAL_OUTPUT_DIR
  COMMAND="python bench/check_bench_regressions.py -a 25th -b $PLATFORM -d $RAW_BENCH_DATA_DIR -e $PLATFORM_DIR/expectations.txt -r $REVISION"
  echo "$COMMAND" >$ACTUAL_OUTPUT_DIR/command_line
  START_TIMESTAMP=$(date +%s)
  $COMMAND 2>$ACTUAL_OUTPUT_DIR/stderr
  echo $? >$ACTUAL_OUTPUT_DIR/return_value
  END_TIMESTAMP=$(date +%s)

  SECONDS_RUN=$(expr $END_TIMESTAMP - $START_TIMESTAMP)
  echo "check_bench_regressions.py took $SECONDS_RUN seconds to complete"

  compare_directories $EXPECTED_OUTPUT_DIR $ACTUAL_OUTPUT_DIR
}

# Test rebaseline.py's JSON-format expectations rebaselining capability.
#
# Copy expected-results.json files from $1 into a dir where they can be modified.
# Run rebaseline.py with arguments in $2, recording its output.
# Then compare the output (and modified expected-results.json files) to the
# content of $2/output-expected.
function rebaseline_test {
  if [ $# != 3 ]; then
    echo "rebaseline_test requires exactly 3 parameters, got $#"
    exit 1
  fi
  COPY_EXPECTATIONS_FROM_DIR="$1"
  ARGS="$2"
  ACTUAL_OUTPUT_DIR="$3/output-actual"
  EXPECTED_OUTPUT_DIR="$3/output-expected"

  rm -rf $ACTUAL_OUTPUT_DIR
  mkdir -p $ACTUAL_OUTPUT_DIR
  EXPECTATIONS_TO_MODIFY_DIR="$ACTUAL_OUTPUT_DIR/gm-expectations"
  BUILDERS=$(ls $COPY_EXPECTATIONS_FROM_DIR)
  for BUILDER in $BUILDERS; do
    mkdir -p $EXPECTATIONS_TO_MODIFY_DIR/$BUILDER
    cp $COPY_EXPECTATIONS_FROM_DIR/$BUILDER/expected-results.json \
       $EXPECTATIONS_TO_MODIFY_DIR/$BUILDER
  done
  COMMAND="python tools/rebaseline.py --expectations-root $EXPECTATIONS_TO_MODIFY_DIR $ARGS"
  echo "$COMMAND" >$ACTUAL_OUTPUT_DIR/command_line
  $COMMAND &>$ACTUAL_OUTPUT_DIR/stdout
  echo $? >$ACTUAL_OUTPUT_DIR/return_value

  compare_directories $EXPECTED_OUTPUT_DIR $ACTUAL_OUTPUT_DIR
}

# Run jsondiff.py with arguments in $1, recording its output.
# Then compare that output to the content of $2/output-expected.
function jsondiff_test {
  if [ $# != 2 ]; then
    echo "jsondiff_test requires exactly 2 parameters, got $#"
    exit 1
  fi
  ARGS="$1"
  ACTUAL_OUTPUT_DIR="$2/output-actual"
  EXPECTED_OUTPUT_DIR="$2/output-expected"

  rm -rf $ACTUAL_OUTPUT_DIR
  mkdir -p $ACTUAL_OUTPUT_DIR
  COMMAND="python tools/jsondiff.py $ARGS"
  echo "$COMMAND" >$ACTUAL_OUTPUT_DIR/command_line
  $COMMAND &>$ACTUAL_OUTPUT_DIR/stdout
  echo $? >$ACTUAL_OUTPUT_DIR/return_value

  compare_directories $EXPECTED_OUTPUT_DIR $ACTUAL_OUTPUT_DIR
}



#
# Run skdiff tests...
#

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

#
# Run bench alerts tests...
#

# Parse a collection of bench data
PLATFORM=Perf-Android-Nexus7-Tegra3-Arm7-Release
REVISION=69c9e1a7261a3c8361e2b2c109d6340862149e34
download_bench_rawdata $PLATFORM $REVISION "$BENCHDATA_FILE_SUFFIXES_NO_INDIVIDUAL_TILES"
download_bench_rawdata $PLATFORM $REVISION "$BENCHDATA_FILE_SUFFIXES_YES_INDIVIDUAL_TILES"
benchalert_test $PLATFORM $REVISION

#
# Run self test for skimage ...
#

COMMAND="python tools/tests/skimage_self_test.py"
echo "$COMMAND"
$COMMAND
ret=$?
if [ $ret -ne 0 ]; then
    echo "skimage self tests failed."
    exit 1
fi

#
# Test rebaseline.py ...
#

REBASELINE_INPUT=tools/tests/rebaseline/input
REBASELINE_OUTPUT=tools/tests/rebaseline/output
rebaseline_test "$REBASELINE_INPUT/json1" "--actuals-base-url $REBASELINE_INPUT/json1 --builders Test-Android-GalaxyNexus-SGX540-Arm7-Debug Test-Win7-ShuttleA-HD2000-x86-Release" "$REBASELINE_OUTPUT/using-json1-expectations"
rebaseline_test "$REBASELINE_INPUT/json1" "--actuals-base-url $REBASELINE_INPUT/json1 --bugs 1234 5678 --builders Test-Android-GalaxyNexus-SGX540-Arm7-Debug Test-Win7-ShuttleA-HD2000-x86-Release --notes notes_content --unreviewed" "$REBASELINE_OUTPUT/marked-unreviewed"
rebaseline_test "$REBASELINE_INPUT/json1" "--actuals-base-url $REBASELINE_INPUT/json1 --add-new --builders Test-Android-GalaxyNexus-SGX540-Arm7-Debug Test-Mac10.6-MacMini4.1-GeForce320M-x86-Release Test-Win7-ShuttleA-HD2000-x86-Release" "$REBASELINE_OUTPUT/add-new"

#
# Test jsondiff.py ...
#

JSONDIFF_INPUT=tools/tests/jsondiff/input
JSONDIFF_OUTPUT=tools/tests/jsondiff/output
jsondiff_test "$JSONDIFF_INPUT/old.json $JSONDIFF_INPUT/new.json" "$JSONDIFF_OUTPUT/old-vs-new"


echo "All tests passed."
