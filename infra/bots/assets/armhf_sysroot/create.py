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
import subprocess
import sys

from distutils import dir_util


def create_asset(target_dir):
  """Create the asset."""

  print "Installing some cross-compiling packages. Hit enter to continue."
  raw_input()
  subprocess.check_call([
    "sudo","apt-get","install",
    "libstdc++-4.8-dev-armhf-cross",
    "libgcc-4.8-dev-armhf-cross",
    "binutils-arm-linux-gnueabihf"
  ])


  shutil.copytree('/usr/arm-linux-gnueabihf', target_dir)
  shutil.copytree('/usr/lib/gcc-cross/arm-linux-gnueabihf/4.8.4',
    os.path.join(target_dir, 'gcc-cross'))
  # copy_tree allows copying into a dir that exists
  # We need to augment the toolchain with some lib*.so that help ld
  # do its magic as well as some includes that may be useful.
  dir_util.copy_tree('/usr/x86_64-linux-gnu/arm-linux-gnueabihf/lib',
                     os.path.join(target_dir, 'lib'))
  dir_util.copy_tree('/usr/x86_64-linux-gnu/arm-linux-gnueabihf/include',
                     os.path.join(target_dir, 'include'))

  # The file paths in libpthread.so and libc.so start off as absolute file
  # paths (e.g. /usr/arm-linux-gnueabihf/lib/libpthread.so.0), which won't
  # work on the bots. We use fileinput to replace just those lines (which
  # start with GROUP). fileinput redirects stdout, so printing here actually
  # writes to the file.
  bad_libpthread = os.path.join(target_dir, "lib", "libpthread.so")
  for line in fileinput.input(bad_libpthread, inplace=True):
    if line.startswith("GROUP"):
      print "GROUP ( libpthread.so.0 libpthread_nonshared.a )"
    else:
      print line

  bad_libc = os.path.join(target_dir, "lib", "libc.so")
  for line in fileinput.input(bad_libc, inplace=True):
    if line.startswith("GROUP"):
      print ("GROUP ( libc.so.6 libc_nonshared.a "
             "AS_NEEDED ( ld-linux-armhf.so.3 ) )")
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
