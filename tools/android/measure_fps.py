#!/usr/bin/env python

# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from __future__ import print_function
import optparse
import re
import subprocess
import time


def query_surfaceflinger_frame_count():
  parcel = subprocess.Popen("adb shell service call SurfaceFlinger 1013",
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            shell=True).communicate()[0]
  if not parcel:
    raise Exception("FAILED: adb shell service call SurfaceFlinger 1013")

  framecount = re.search("Result: Parcel\(([a-f0-9]+) ", parcel)
  if not framecount:
    raise Exception("Unexpected result from SurfaceFlinger: " + parcel)

  return int(framecount.group(1), 16)


def main(interval):
  startframe = query_surfaceflinger_frame_count()
  starttime = time.time()

  while True:
    time.sleep(interval)

    endframe = query_surfaceflinger_frame_count()
    endtime = time.time()
    fps = (endframe - startframe) / (endtime - starttime)
    print("%.2f" % fps)

    startframe = endframe
    starttime = endtime


if __name__ == '__main__':
  parser = optparse.OptionParser()
  parser.add_option("-i", "--interval", type="int", default="2",
                    help="Number of seconds to count frames.")
  options, args = parser.parse_args()
  main(options.interval)

