#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import common
import os
import subprocess
import utils


# Copied from CLANG_REVISION here:
# https://cs.chromium.org/chromium/src/tools/clang/scripts/update.py
CLANG_REVISION = '357692'
CLANG_SUB_REVISION = '1'
CLANG_PKG_VERSION = '%s-%s' % (CLANG_REVISION, CLANG_SUB_REVISION)
GS_URL = ('https://commondatastorage.googleapis.com/chromium-browser-clang'
          '/Win/clang-%s.tgz' % CLANG_PKG_VERSION)


def create_asset(target_dir):
  """Create the asset."""
  with utils.chdir(target_dir):
    tarball = 'clang.tgz'
    subprocess.check_call(['wget', '-O', tarball, GS_URL])
    subprocess.check_call(['tar', 'zxvf', tarball])
    os.remove(tarball)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
