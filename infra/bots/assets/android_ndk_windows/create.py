#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import glob
import os.path
import shutil
import subprocess

NDK_VER = "android-ndk-r19b"
NDK_URL = \
    "https://dl.google.com/android/repository/%s-windows-x86_64.zip" % NDK_VER

def create_asset(target_dir):
  """Create the asset."""
  subprocess.check_call(["curl", NDK_URL, "-o", "ndk.zip"])
  subprocess.check_call(["unzip", "ndk.zip", "-d", target_dir])
  for f in glob.glob(os.path.join(target_dir, NDK_VER, "*")):
    shutil.move(f, target_dir)
  subprocess.check_call(["rm", "ndk.zip"])

  # Some of these files have paths that exceed 260 characters when downloaded
  # as a CIPD package.  Luckily they're just tests.  We don't need them.
  shutil.rmtree(os.path.join(target_dir,
                             'sources', 'cxx-stl', 'llvm-libc++', 'test'))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
