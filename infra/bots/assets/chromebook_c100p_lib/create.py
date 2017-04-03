#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import glob
import os
import shutil


def create_asset(target_dir, lib_path):
  """Create the asset."""
  for f in glob.glob(os.path.join(lib_path,'libGL*')):
    shutil.copy(f, target_dir)

  for f in glob.glob(os.path.join(lib_path,'libEGL*')):
    shutil.copy(f, target_dir)

  for f in glob.glob(os.path.join(lib_path,'libmali*')):
    shutil.copy(f, target_dir)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  parser.add_argument('--lib_path', '-l', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir, args.lib_path)


if __name__ == '__main__':
  main()
