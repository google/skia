#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import subprocess


ZIP_URL = ('https://github.com/google/protobuf/releases/download/v3.3.0/'
           'protoc-3.3.0-linux-x86_64.zip')


def create_asset(target_dir):
  """Create the asset."""
  local_zip = '/tmp/protoc.zip'
  subprocess.check_call(['curl', '-L', ZIP_URL, '-o', local_zip])
  subprocess.check_call(['unzip', local_zip, '-d', target_dir])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
