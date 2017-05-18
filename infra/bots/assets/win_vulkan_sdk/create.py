#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import shutil
import sys
import os


def create_asset(target_dir, sdk_path, runtime_path):
  """Create the asset."""
  if not os.path.isdir(target_dir):
    os.makedirs(target_dir)
  shutil.copytree(sdk_path, target_dir)
  shutil.copyfile(runtime_path, os.path.join(target_dir, "vulkan-1.dll"))

def main():
  if sys.platform != 'win32':
    print >> sys.stderr, 'This script only runs on Windows.'
    sys.exit(1)
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  parser.add_argument('--sdk_path', '-s', required=True)
  parser.add_argument('--runtime_path', '-r',
      default=os.path.join("C:","System32","vulkan-1.dll"),
      required=True)
  args = parser.parse_args()
  create_asset(args.target_dir, args.sdk_path, args.runtime_path)


if __name__ == '__main__':
  main()
