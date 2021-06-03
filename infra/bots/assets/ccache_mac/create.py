#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create a ccache binary for mac hosts."""


import argparse
import os
import subprocess
import sys

FILE_DIR = os.path.dirname(os.path.abspath(__file__))
INFRA_BOTS_DIR = os.path.realpath(os.path.join(FILE_DIR, os.pardir, os.pardir))
sys.path.insert(0, INFRA_BOTS_DIR)
import utils


URL = "https://github.com/ccache/ccache/releases/download/v3.7.7/ccache-3.7.7.tar.gz"
VERSION = "ccache-3.7.7"


def create_asset(target_dir):
  # configure --prefix requires an absolute path.
  target_dir = os.path.abspath(target_dir)

  # Download and extract the source.
  with utils.tmp_dir():
    subprocess.check_call(["curl", "-L", "-o", VERSION + ".tar.gz",
                           "https://github.com/ccache/ccache/releases/download/v3.7.7/ccache-3.7.7.tar.gz"])
    subprocess.check_call(["tar", "-xzf", VERSION + ".tar.gz"])
    os.chdir(VERSION)

    subprocess.check_call(["./configure", "--disable-man", "--prefix=" + target_dir])
    subprocess.check_call(["make"])
    subprocess.check_call(["make" ,"install"])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
