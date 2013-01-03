#!/bin/bash

# Rebaseline the skdiff/*/output-expected/ subdirectories used by the skdiff
# self-tests.
# Use with caution: are you sure the new results are actually correct?
#
# YOU MUST RE-RUN THIS UNTIL THE SELF-TESTS SUCCEED!
#
# TODO: currently, this must be run on Linux to generate baselines that match
# the ones on the housekeeper bot (which runs on Linux... see
# http://70.32.156.51:10117/builders/Skia_PerCommit_House_Keeping/builds/1417/steps/RunGmSelfTests/logs/stdio )
# See https://code.google.com/p/skia/issues/detail?id=677
# ('make tools/tests/run.sh work cross-platform')

function replace_expected_with_actual {
  # Delete all the expected output files
  EXPECTED_FILES=$(find skdiff/*/output-expected -type f | grep -v /\.svn/)
  for EXPECTED_FILE in $EXPECTED_FILES; do
    rm $EXPECTED_FILE
  done

  # Copy all the actual output files into the "expected" directories,
  # creating new subdirs as we go.
  ACTUAL_FILES=$(find skdiff/*/output-actual -type f | grep -v /\.svn/)
  for ACTUAL_FILE in $ACTUAL_FILES; do
    EXPECTED_FILE=${ACTUAL_FILE//actual/expected}
    mkdir -p $(dirname $EXPECTED_FILE)
    cp $ACTUAL_FILE $EXPECTED_FILE
  done
}

function svn_add_new_files {
  # Delete all the "actual" directories, so we can svn-add any new "expected"
  # directories without adding the "actual" ones.
  rm -rf skdiff/*/output-actual
  FILES=$(svn stat skdiff/* | grep ^\? | awk '{print $2}')
  for FILE in $FILES; do
    svn add $FILE
  done
  FILES=$(svn stat skdiff/*/output-expected | grep ^\? | awk '{print $2}')
  for FILE in $FILES; do
    svn add $FILE
  done
}

function svn_delete_old_files {
  FILES=$(svn stat skdiff/*/output-expected | grep ^\! | awk '{print $2}')
  for FILE in $FILES; do
    svn rm $FILE
  done
  FILES=$(svn stat skdiff/* | grep ^\! | awk '{print $2}')
  for FILE in $FILES; do
    svn rm $FILE
  done
}


# cd into the gm self-test dir
cd $(dirname $0)

./run.sh
SELFTEST_RESULT=$?
echo
if [ "$SELFTEST_RESULT" != "0" ]; then
  replace_expected_with_actual
  echo "Self-tests still failing, you should probably run this again..."
else
  svn_add_new_files
  svn_delete_old_files
  echo "Self-tests succeeded this time, you should be done!"
fi
exit $SELFTEST_RESULT

