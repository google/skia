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

PKGS = [
    'binutils-mips64el-linux-gnuabi64',
    'cpp-6-mips64el-linux-gnuabi64',
    'g++-6-mips64el-linux-gnuabi64',
    'gcc-6-cross-base',
    'gcc-6-mips64el-linux-gnuabi64',
    'gcc-6-mips64el-linux-gnuabi64-base',
    'libatomic1-mips64el-cross',
    'libc6-dev-mips64el-cross',
    'libc6-mips64el-cross',
    'libgcc-6-dev-mips64el-cross',
    'libgcc1-mips64el-cross',
    'libgomp1-mips64el-cross',
    'libstdc++-6-dev-mips64el-cross',
    'libstdc++6-mips64el-cross',
    'linux-libc-dev-mips64el-cross',
]

def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    # Download required Debian packages.
    subprocess.check_call(['apt-get', 'download'] + PKGS)
    # This is a bit hacky. Rather than installing to a chroot, just extract all
    # the packages to the target dir.
    for f in os.listdir('.'):
      subprocess.check_call(['dpkg-deb', '--extract', f, target_dir])
  parent_dir = os.path.join(target_dir, 'usr')
  # Remove unnecessary files that cause problems with zipping (due to dangling
  # symlinks).
  os.remove(os.path.join(parent_dir,
                         'lib/gcc-cross/mips64el-linux-gnuabi64/6/libcc1.so'))
  shutil.rmtree(os.path.join(parent_dir, 'share'))
  # Remove usr/ prefix.
  for d in os.listdir(parent_dir):
    os.rename(os.path.join(parent_dir, d), os.path.join(target_dir, d))
  os.rmdir(parent_dir)

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
