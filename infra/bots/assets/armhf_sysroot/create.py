#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import fileinput
import os
import shutil
import sys


def create_asset(target_dir):
  """Create the asset."""
  shutil.copytree('/usr/arm-linux-gnueabihf', target_dir)
  shutil.copytree('/usr/lib/gcc-cross/arm-linux-gnueabihf/4.8.4',
    os.path.join(target_dir, 'gcc-cross'))

  # Fix absolute file paths in libpthread and libc to be relative paths
  bad_libpthread = os.path.join(target_dir, "lib", "libpthread.so")
  for line in fileinput.input(bad_libpthread, inplace=True):
    if line.startswith("GROUP"):
      print "GROUP ( libpthread.so.0 libpthread_nonshared.a )"
    else:
      print line

  bad_libc = os.path.join(target_dir, "lib", "libc.so")
  for line in fileinput.input(bad_libc, inplace=True):
    if line.startswith("GROUP"):
      print "GROUP ( libc.so.6 libc_nonshared.a  AS_NEEDED ( ld-linux-armhf.so.3 ) )"
    else:
      print line


def main():
  if 'linux' not in sys.platform:
    print >> sys.stderr, 'This script only runs on Linux.'
    sys.exit(1)
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
