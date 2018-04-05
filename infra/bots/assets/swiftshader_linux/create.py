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
import shutil
import subprocess
import utils


# Download the pre-built SwiftShader libs:
# The more recent pre-built are here:
# https://storage.googleapis.com/swiftshader-binaries/index.html
BINARIES_URL = ('https://storage.googleapis.com/swiftshader-binaries/OpenGL_ES/'
                'Latest/Linux/')
BINARIES = [
    'libGLESv2.so',
    'libEGL.so',
]
REPO_URL = 'https://swiftshader.googlesource.com/SwiftShader'


def create_asset(target_dir):
  """Create the asset."""
  for b in BINARIES:
    subprocess.check_call(["curl", BINARIES_URL + b, "-o",
                           os.path.join(target_dir, b)])
  with utils.tmp_dir():
    subprocess.check_call(["git", "clone", "--depth", "1", REPO_URL, "."])
    shutil.copytree("./include", os.path.join(target_dir, "include"))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
