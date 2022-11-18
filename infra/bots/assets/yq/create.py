#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import subprocess


DOWNLOAD_URL = 'https://github.com/mikefarah/yq/releases/download/v4.30.4/yq_linux_amd64'


def create_asset(target_dir):
  """Create the asset."""
  subprocess.check_call(['wget', '-O', 'yq', DOWNLOAD_URL], cwd=target_dir)
  subprocess.check_call(['chmod', 'a+x', 'yq'], cwd=target_dir)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()

