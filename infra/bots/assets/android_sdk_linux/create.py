#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import os
import shutil
import sys


ENV_VAR = 'ANDROID_SDK_LINUX_SDK_ROOT'


def getenv(key):
  val = os.environ.get(key)
  if not val:
    print(('Environment variable %s not set; you should run this via '
           'create_and_upload.py.' % key), file=sys.stderr)
    sys.exit(1)
  return val


def create_asset(target_dir, android_sdk_root):
  """Create the asset."""
  dst = os.path.join(target_dir, 'android-sdk')
  shutil.copytree(android_sdk_root, dst)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  android_sdk_root = getenv(ENV_VAR)
  create_asset(args.target_dir, android_sdk_root)


if __name__ == '__main__':
  main()
