#!/usr/bin/env python
#
# Copyright 2018 Google Inc.
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

# The OpenCL C headers are available at
# https://github.com/KhronosGroup/OpenCL-Headers, but the C++ header cl.hpp
# would need to be generated from https://github.com/KhronosGroup/OpenCL-CLHPP.
# Instead, we just grab the pre-built headers from the Debian packages.
PKGS = [
    'opencl-c-headers',
    'opencl-clhpp-headers',
]

def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    # Download required Debian packages.
    subprocess.check_call(['apt-get', 'download'] + PKGS)
    # Extract to CWD.
    for f in os.listdir('.'):
      subprocess.check_call(['dpkg-deb', '--extract', f, '.'])
    # Copy usr/include/CL to target_dir.
    shutil.move(os.path.join(os.getcwd(), 'usr', 'include', 'CL'), target_dir)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
