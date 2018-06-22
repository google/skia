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

PKGS = [
    # Use libOpenCL.so from the ocl-icd-opencl-dev and ocl-icd-libopencl1 Debian
    # packages.
    'ocl-icd-opencl-dev',
    'ocl-icd-libopencl1',
    # Include the Beignet driver for Intel.
    'beignet-opencl-icd',
]

def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    # Download required Debian packages.
    subprocess.check_call(['apt-get', 'download'] + PKGS)
    # Extract to CWD.
    for f in os.listdir('.'):
      subprocess.check_call(['dpkg-deb', '--extract', f, '.'])
    # Move usr/lib/x86_64-linux-gnu/* to target_dir.
    lib_dir = os.path.join(os.getcwd(), 'usr', 'lib', 'x86_64-linux-gnu')
    for f in os.listdir(lib_dir):
      shutil.move(os.path.join(lib_dir, f), target_dir)
    # Validate assumptions below.
    with open(os.path.join(os.getcwd(), 'etc', 'OpenCL', 'vendors',
                           'intel-beignet-x86_64-linux-gnu.icd')) as icd:
      expected = '/usr/lib/x86_64-linux-gnu/beignet/libcl.so'
      actual = os.path.normpath(icd.readlines()[0].strip())
      if expected != actual:
        raise Exception('Expected beignet ICD to point to %s, but it now points'
                        'to %s; please update create.py.' % (expected, actual))

  # Generate vendors dir.
  os.mkdir(os.path.join(target_dir, 'vendors'))
  # Currently only Beignet.
  with open(os.path.join(target_dir, 'vendors', 'beignet.icd'), 'w') as icd:
    icd.write('beignet/libcl.so\n')


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
