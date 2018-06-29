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

# Use libOpenCL.so from the ocl-icd-opencl-dev Debian package.
PKGS = [
    'ocl-icd-opencl-dev',
    'ocl-icd-libopencl1',
]

def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    # Download required Debian packages.
    subprocess.check_call(['apt-get', 'download'] + PKGS)
    # Extract to CWD.
    for f in os.listdir('.'):
      subprocess.check_call(['dpkg-deb', '--extract', f, '.'])
    # Copy usr/lib/x86_64-linux-gnu/* to target_dir.
    lib_dir = os.path.join(os.getcwd(), 'usr', 'lib', 'x86_64-linux-gnu')
    for f in os.listdir(lib_dir):
      shutil.move(os.path.join(lib_dir, f), target_dir)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
