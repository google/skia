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


def create_asset(target_dir, sdk_path):
  """Create the asset."""
  # The bots only need Include from the SDK.
  target_include_dir = os.path.join(target_dir, "Include")
  sdk_include_dir = os.path.join(sdk_path, "Include")
  if not os.path.isdir(target_dir):
    os.makedirs(target_dir)
  shutil.copytree(sdk_include_dir, target_include_dir)

def main():
  if sys.platform != 'win32':
    print >> sys.stderr, 'This script only runs on Windows.'
    sys.exit(1)
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  parser.add_argument('--sdk_path', '-s', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir, args.sdk_path)


if __name__ == '__main__':
  main()
