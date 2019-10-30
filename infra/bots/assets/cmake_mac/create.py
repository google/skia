#!/usr/bin/env python
#
# Copyright 2019 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import common
import subprocess
import utils

VERSION = '3.13.4'
URL = ('https://github.com/Kitware/CMake/releases/download/v%s/'
       'cmake-%s-Darwin-x86_64.tar.gz') % (VERSION, VERSION)


def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    subprocess.check_call(['wget', URL, '--output-document=cmake.tar.gz'])
    subprocess.check_call(['tar', '--extract', '--gunzip', '--file',
                           'cmake.tar.gz', '--directory', target_dir,
                           '--strip-components', '1'])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
