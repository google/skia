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


# Copied from https://cs.chromium.org/chromium/src/tools/clang/scripts/update.py
CLANG_REVISION = '0e41d647ceaeb2195d5d9ab5ff25c19292a36bf5'
CLANG_SVN_REVISION = 'n354867'
CLANG_SUB_REVISION = 2

PACKAGE_VERSION = '%s-%s-%s' % (CLANG_SVN_REVISION, CLANG_REVISION[:8],
                                CLANG_SUB_REVISION)
# (End copying)

GS_URL = ('https://commondatastorage.googleapis.com/chromium-browser-clang'
          '/Win/clang-%s.tgz' % PACKAGE_VERSION)


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
