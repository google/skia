#!/usr/bin/env python
#
# Copyright 2018 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import common
import multiprocessing
import os
import shutil
import subprocess
import utils

PKGS = [
    # Use libOpenCL.so from the ocl-icd-opencl-dev and ocl-icd-libopencl1 Debian
    # packages.
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
    # Move usr/lib/x86_64-linux-gnu/* to target_dir.
    lib_dir = os.path.join(os.getcwd(), 'usr', 'lib', 'x86_64-linux-gnu')
    for f in os.listdir(lib_dir):
      shutil.move(os.path.join(lib_dir, f), target_dir)

  # Check out and build Beignet. Following instructions here:
  # https://www.freedesktop.org/wiki/Software/Beignet/
  with utils.tmp_dir():
    # Install build deps. Note use of clang-3.9, libclang-3.9-dev, and llvm-3.9
    # so that the Beignet lib works on Debian 9.
    beignet_build_deps = ['clang-3.9', 'cmake', 'libclang-3.9-dev',
                          'libdrm-dev', 'libedit-dev', 'libegl1-mesa-dev',
                          'libtinfo-dev', 'libxext-dev', 'libxfixes-dev',
                          'llvm-3.9', 'ocl-icd-dev', 'ocl-icd-opencl-dev',
                          'pkg-config', 'python', 'zlib1g-dev']
    apt_get_cmd = ['sudo', 'apt-get', 'install'] + beignet_build_deps
    print 'Running "%s"' % ' '.join(apt_get_cmd)
    subprocess.check_call(apt_get_cmd)
    # Check out repo.
    subprocess.check_call(['git', 'clone', '--depth', '1',
                           'https://anongit.freedesktop.org/git/beignet.git'])
    # Configure the build.
    build_dir = os.path.join(os.getcwd(), 'beignet', 'build')
    os.mkdir(build_dir)
    os.chdir(build_dir)
    # We override the default BEIGNET_INSTALL_DIR because we don't control where
    # this asset is placed on the filesystem. Using a relative path here causes
    # Beignet to load its sub-libraries from a path relative to LD_LIBRARY_PATH.
    subprocess.check_call(['cmake', '-DBEIGNET_INSTALL_DIR=beignet/', '../'])
    # Build and package the library.
    subprocess.check_call(['make', '-j%d' % multiprocessing.cpu_count(),
                           'package'])
    # Extract library.
    tarfile = 'Beignet-1.4.-Linux.tar.gz'
    match = 'Beignet-1.4.-Linux/usr/local/beignet'
    strip_components = len(match.split('/'))-1  # Keep beignet.
    subprocess.check_call(['tar', 'xzf', tarfile,
                           '--directory', target_dir,
                           '--strip-components=%d' % strip_components,
                           '--anchored', match])

  # Generate vendors dir.
  os.mkdir(os.path.join(target_dir, 'vendors'))
  # Currently only Beignet.
  with open(os.path.join(target_dir, 'vendors', 'beignet.icd'), 'w') as icd:
    # Use relative path, which will be resolved by LD_LIBRARY_PATH.
    icd.write('beignet/libcl.so\n')


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
