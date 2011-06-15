#!/bin/sh
#
# Run a sequence of gamma tests quietly
err=0

echo >> pngtest-log.txt
echo "============ pngvalid-full.sh ==============" >> pngtest-log.txt

echo "Running test-pngvalid-full.sh"
for gamma in threshold transform sbit 16-to-8
do
   if ./pngvalid "--gamma-$gamma" >> pngtest-log.txt 2>&1
   then
      echo "  PASS:" pngvalid "--gamma-$gamma"
   else
      echo "  FAIL:" pngvalid "--gamma-$gamma"
      err=1
   fi
done

echo

exit $err
