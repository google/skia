#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re
import subprocess
import sys

"""Prints the lowest locally available SDK version greater than or equal to a
given minimum sdk version to standard output.

Usage:
  python find_sdk.py 10.6  # Ignores SDKs < 10.6
"""

def parse_version(version_str):
  """'10.6' => [10, 6]"""
  return map(int, re.findall(r'(\d+)', version_str))

def find_sdk_dir():
  job = subprocess.Popen(['xcode-select', '-print-path'],
                         stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT)
  out, err = job.communicate()
  if job.returncode != 0:
    print >>sys.stderr, out
    print >>sys.stderr, err
    raise Exception(('Error %d running xcode-select, you might have to run '
      '|sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer| '
      'if you are using Xcode 4.') % job.returncode)
  # The Developer folder moved in Xcode 4.3.
  xcode43_sdk_path = os.path.join(
      out.rstrip(), 'Platforms/MacOSX.platform/Developer/SDKs')
  if os.path.isdir(xcode43_sdk_path):
    sdk_dir = xcode43_sdk_path
  else:
    sdk_dir = os.path.join(out.rstrip(), 'SDKs')
    
  return sdk_dir

def main():
  min_sdk_version = '10.6'
  if len(sys.argv) > 1:
    min_sdk_version = sys.argv[1]

  sdk_dir = find_sdk_dir()

  sdks = [re.findall('^MacOSX(10\.\d+)\.sdk$', s) for s in os.listdir(sdk_dir)]
  sdks = [s[0] for s in sdks if s]  # [['10.5'], ['10.6']] => ['10.5', '10.6']
  sdks = [s for s in sdks  # ['10.5', '10.6'] => ['10.6']
          if parse_version(s) >= parse_version(min_sdk_version)]
  if not sdks:
    raise Exception('No %s+ SDK found' % min_sdk_version)
  best_sdk = min(sdks, key=parse_version)

  return best_sdk

if __name__ == '__main__':
  if sys.platform != 'darwin':
    raise Exception("This script only runs on Mac")
  print main()
