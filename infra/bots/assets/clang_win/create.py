#!/usr/bin/env python
#
# Copyright 2017 Google LLC
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


# From a Chromium checkout, run the following script and look for the file that it downloads.
# $ tools/clang/scripts/update.py --output-dir /tmp/win_clang --host-os win
# Downloading https://commondatastorage.googleapis.com/chromium-browser-clang/Win/$TAR_FILE

TAR_FILE = "clang-llvmorg-23-init-10931-g20b6ec66-11.tar.xz"
GS_URL = f'https://commondatastorage.googleapis.com/chromium-browser-clang/Win/{TAR_FILE}'

def create_asset(target_dir):
  """Create the asset."""
  with utils.chdir(target_dir):
    tarball = 'clang.tar.xz'
    subprocess.check_call(['wget', '-O', tarball, GS_URL])
    subprocess.check_call(['tar', 'xf', tarball])
    os.remove(tarball)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
