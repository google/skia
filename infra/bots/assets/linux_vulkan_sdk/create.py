#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
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


SDK_VERSION='1.2.141.0'
SDK_URL=('https://sdk.lunarg.com/sdk/download/%s/linux/'
         'vulkansdk-linux-x86_64-%s.tar.gz' % (SDK_VERSION, SDK_VERSION))


def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    tarball = 'vulkansdk-linux.tar.gz'
    subprocess.check_call(['curl', SDK_URL, '--output', tarball])
    subprocess.check_call(['tar', '--extract', '--verbose',
                           '--file=%s' % tarball, '--gunzip',
                           '--directory=%s' % target_dir,
                           '--strip-components=2',
                           '%s/x86_64' % SDK_VERSION])


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
