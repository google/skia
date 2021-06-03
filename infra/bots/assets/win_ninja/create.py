#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import subprocess


VERSION = "v1.8.2"
URL = "https://github.com/ninja-build/ninja/releases/download/%s/ninja-win.zip"


def create_asset(target_dir):
  """Create the asset."""
  subprocess.check_call(["curl", "-L", URL % VERSION, "-o", "ninja-win.zip"])
  subprocess.check_call(["unzip", "ninja-win.zip", "-d", target_dir])
  subprocess.check_call(["rm", "ninja-win.zip"])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
