#!/bin/bash

# Rebaseline the outputs/*/output-expected/ subdirectories used by the
# gm self-tests.
# Use with caution: are you sure the new results are actually correct?
#
# YOU MUST RE-RUN THIS UNTIL THE SELF-TESTS SUCCEED!
# (It takes one run for each call to gm_test in run.sh)

# cd into the gm self-test dir
cd $(dirname $0)

./run.sh

# Delete all the expected output files
EXPECTED_FILES=$(find outputs/*/output-expected -type f | grep -v /\.svn/)
for EXPECTED_FILE in $EXPECTED_FILES; do
  rm $EXPECTED_FILE
done

# Copy all the actual output files into the "expected" directories,
# creating new subdirs as we go.
ACTUAL_FILES=$(find outputs/*/output-actual -type f | grep -v /\.svn/)
for ACTUAL_FILE in $ACTUAL_FILES; do
  EXPECTED_FILE=${ACTUAL_FILE//actual/expected}
  mkdir -p $(dirname $EXPECTED_FILE)
  cp $ACTUAL_FILE $EXPECTED_FILE
done

# "svn add" any newly expected files/dirs, and "svn rm" any that are gone now
FILES=$(svn stat outputs/*/output-expected | grep ^\? | awk '{print $2}')
for FILE in $FILES; do
  svn add $FILE
done
FILES=$(svn stat outputs/*/output-expected | grep ^\! | awk '{print $2}')
for FILE in $FILES; do
  svn rm $FILE
done
