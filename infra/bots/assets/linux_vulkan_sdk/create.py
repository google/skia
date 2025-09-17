#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import hashlib
import os
import subprocess
import sys

FILE_DIR = os.path.dirname(os.path.abspath(__file__))
INFRA_BOTS_DIR = os.path.realpath(os.path.join(FILE_DIR, os.pardir, os.pardir))
sys.path.insert(0, INFRA_BOTS_DIR)
import utils

# https://vulkan.lunarg.com/sdk/home
SDK_VERSION='1.4.321.1'
SDK_SHA256='f22a3625bd4d7a32e7a0d926ace16d5278c149e938dac63cecc00537626cbf73'
SDK_URL=('https://sdk.lunarg.com/sdk/download/%s/linux/'
         'vulkansdk-linux-x86_64-%s.tar.xz' % (SDK_VERSION, SDK_VERSION))


def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    tarball = 'vulkansdk-linux.tar.xz'
    # Use -L to follow redirects.
    subprocess.check_call(['curl', '-L', SDK_URL, '--output', tarball])

    # Verify the SHA256 hash.
    with open(tarball, 'rb') as f:
      actual_hash = hashlib.sha256(f.read()).hexdigest()
    if actual_hash != SDK_SHA256:
      raise Exception('SHA256 mismatch for %s. Expected %s, got %s' %
                      (tarball, SDK_SHA256, actual_hash))

    subprocess.check_call(['tar', '--extract', '--verbose',
                           '--file=%s' % tarball,
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
