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

def create_asset(target_dir, lib_path):
  """Create the asset."""
  shutil.copytree(lib_path, target_dir)


def main():
  if 'linux' not in sys.platform:
    print >> sys.stderr, 'This script only runs on Linux.'
    sys.exit(1)
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  parser.add_argument('--lib_path', '-l', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir, args.lib_path)


if __name__ == '__main__':
  main()
