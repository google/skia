#!/bin/sh
#
# Run a sequence of tests quietly, without the slow
# gamma tests
err=0

echo >> pngtest-log.txt
echo "============ pngvalid-simple.sh ==============" >> pngtest-log.txt
echo "Running test-pngvalid-simple.sh"
# The options to test are:
#
# standard tests with and without progressive reading and interlace
# size images with and without progressive reading
# transform tests (standard, non-interlaced only)
#
for opts in "--standard" "--standard --progressive-read" \
   "--standard --interlace" "--standard --progressive-read --interlace" \
   "--size" "--size --progressive-read" \
   "--transform"
do
   if ./pngvalid  $opts >> pngtest-log.txt 2>&1
   then
      echo "  PASS:" pngvalid $opts
   else
      echo "  FAIL:" pngvalid $opts
      err=1
   fi
done

exit $err
