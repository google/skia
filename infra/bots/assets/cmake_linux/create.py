#!/usr/bin/env python
#
# Copyright 2018 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import os
import subprocess
import sys

FILE_DIR = os.path.dirname(os.path.abspath(__file__))
INFRA_BOTS_DIR = os.path.realpath(os.path.join(FILE_DIR, os.pardir, os.pardir))
sys.path.insert(0, INFRA_BOTS_DIR)
import utils


VERSION = '3.31.8'
URL = ('https://github.com/Kitware/CMake/releases/download/v%s/'
       'cmake-%s-linux-x86_64.tar.gz') % (VERSION, VERSION)


def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    subprocess.check_call(['wget', URL, '--output-document=cmake.tar.gz'])
    subprocess.check_call(['tar', '--extract', '--gunzip', '--file',
                           'cmake.tar.gz'])
    cmake_dir = 'cmake-%s-linux-x86_64' % VERSION
    # Move the bin and share directories into the target directory.
    for d in ['bin', 'share']:
      subprocess.check_call(
          ['mv', os.path.join(cmake_dir, d), target_dir])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
