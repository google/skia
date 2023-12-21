#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import os
import platform
import shutil
import subprocess
import sys


NDK_VER = "android-ndk-r26b"
NDK_URL = \
    "https://dl.google.com/android/repository/%s-darwin.dmg" % NDK_VER
DMG = "ndk.dmg"
MOUNTED_NAME_START = '/Volumes/Android NDK'

def find_ndk(volume):
  """Find the NDK within the mounted volume."""
  for f in os.listdir(volume):
    if f.endswith('.app'):
      return os.path.join(volume, f, 'Contents/NDK')

def create_asset(target_dir):
  """Create the asset."""
  if platform.system() != 'Darwin':
    print("This script can only be run on a Mac!")
    sys.exit(1)

  subprocess.check_call(["curl", NDK_URL, "-o", DMG])
  output = subprocess.check_output(['hdiutil', 'attach', DMG])

  # hdiutil mounted the DMG somewhere - find where it was mounted.
  lines = output.decode('utf-8').split('\n')
  found = False
  for line in lines:
    words = line.split('\t')
    if len(words) == 3:
      if words[2].startswith(MOUNTED_NAME_START):
        found = True

        # copytree (in python2, and by default in python3) requires that the
        # dst does not exist. Remove it so that is the case.
        if os.path.isdir(target_dir):
          os.rmdir(target_dir)

        shutil.copytree(find_ndk(words[2]), target_dir)

        # Unmount the volume, now that we're done with it.
        subprocess.check_call(['hdiutil', 'detach', words[0].strip()])

        subprocess.check_call(["rm", DMG])
        break

  if not found:
    print("Could not find mount point! Output from hdiutil attach:")
    for line in lines:
      print(line)
    sys.exit(2)

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
