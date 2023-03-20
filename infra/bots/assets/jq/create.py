#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import subprocess


DOWNLOAD_URL = 'https://github.com/stedolan/jq/releases/download/jq-1.6/jq-linux64'


def create_asset(target_dir):
  """Create the asset."""
  subprocess.check_call(['wget', '-O', 'jq', DOWNLOAD_URL], cwd=target_dir)
  subprocess.check_call(['chmod', 'a+x', 'jq'], cwd=target_dir)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()

