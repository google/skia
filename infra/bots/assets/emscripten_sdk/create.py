#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import shutil
import sys


def create_asset(target_dir, sdk_dir):
  """Create the asset."""
  shutil.copytree(sdk_dir, target_dir)


def main():
  if 'linux' not in sys.platform:
    print >> sys.stderr, 'This script only runs on Linux.'
    sys.exit(1)
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  parser.add_argument('--sdk_dir', '-s', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir, args.sdk_dir)


if __name__ == '__main__':
  main()
