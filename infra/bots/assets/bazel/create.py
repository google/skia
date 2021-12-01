#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
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

URL = 'https://github.com/bazelbuild/bazel/releases/download/4.1.0/bazel-4.1.0-installer-linux-x86_64.sh'
INSTALLER_SCRIPT = URL.split('/')[-1]


def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    subprocess.call(['wget', URL])
    subprocess.call(['sh', INSTALLER_SCRIPT, '--prefix=' + target_dir])


def main():
  if 'linux' not in sys.platform:
    print('This script only runs on Linux.', file=sys.stderr)
    sys.exit(1)
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
