#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""
Create an updated VS toolchain

Before you can run this script, you need a collated VC toolchain + Windows SDK.
To generate that, run depot_tools/win_toolchain/package_from_installed.py
That script pulls all of the compiler and SDK bits from your locally installed
version of Visual Studio. The comments in that script include instructions on
which components need to be installed (C++, ARM64, etc...)

That script produces a .zip file with a SHA filename. Unzip that file, then
pass the unzipped directory as the src_dir to this script.
"""

import argparse
import common
import os
import shlex
import shutil
import subprocess
import sys
import utils


# By default the toolchain includes a bunch of unnecessary stuff with long path
# names. Trim out directories with these names.
IGNORE_LIST = [
  'WindowsMobile',
  'App Certification Kit',
  'Debuggers',
  'Extension SDKs',
  'DesignTime',
  'AccChecker',
]

def filter_toolchain_files(dirname, files):
  """Callback for shutil.copytree. Return lists of files to skip."""
  split = dirname.split(os.path.sep)
  for ign in IGNORE_LIST:
    if ign in split:
       print 'Ignoring dir %s' % dirname
       return files
  return []

def main():
  if sys.platform != 'win32':
    print >> sys.stderr, 'This script only runs on Windows.'
    sys.exit(1)

  parser = argparse.ArgumentParser()
  parser.add_argument('--src_dir', '-s', required=True)
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  src_dir = os.path.abspath(args.src_dir)
  target_dir = os.path.abspath(args.target_dir)
  shutil.copytree(src_dir, target_dir, ignore=filter_toolchain_files)

if __name__ == '__main__':
  main()
