#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import os
import shutil


def create_asset(target_dir, android_sdk_root):
  """Create the asset."""
  if not android_sdk_root:
    android_sdk_root = (os.environ.get('ANDROID_HOME') or
                        os.environ.get('ANDROID_SDK_ROOT'))
  if not android_sdk_root:
    raise Exception('No --android_sdk_root provided and no ANDROID_HOME or '
                    'ANDROID_SDK_ROOT environment variables.')

  dst = os.path.join(target_dir, 'android-sdk')
  shutil.copytree(android_sdk_root, dst)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--android_sdk_root')
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir, args.android_sdk_root)


if __name__ == '__main__':
  main()
