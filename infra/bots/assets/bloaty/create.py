#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create Bloaty as a Linux executable."""


import argparse
import os
import shutil
import subprocess
import sys

FILE_DIR = os.path.dirname(os.path.abspath(__file__))
INFRA_BOTS_DIR = os.path.realpath(os.path.join(FILE_DIR, os.pardir, os.pardir))
sys.path.insert(0, INFRA_BOTS_DIR)
import utils


REPO = 'https://github.com/google/bloaty'
TAG = 'v1.0'


def create_asset(target_dir):
  with utils.tmp_dir():
    # Check out bloaty
    subprocess.check_call(['git', 'clone', '--depth', '1', '-b', TAG,
                           '--single-branch', REPO])
    os.chdir('bloaty')
    # Build bloaty
    subprocess.check_call(['cmake', '.'])
    subprocess.check_call(['make', '-j'])

    shutil.move('./bloaty', target_dir)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
