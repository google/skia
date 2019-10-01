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

# This is basically all the deps of g++-multilib-mips64el-linux-gnuabi64 that
# are not already installed on the bots.
#
# We could try to also include packages that *are* already installed on the bots
# as well, but that would be quite a bit, and would probably entail more hacky
# fixes like below.
#
# There is probably a way to generate this list from apt, but it's not as
# straightforward as it should be.
PKGS = [
    'binutils-mips64el-linux-gnuabi64',
    'cpp-8-mips64el-linux-gnuabi64',
    'g++-8-mips64el-linux-gnuabi64',
    'gcc-8-cross-base',
    'gcc-8-mips64el-linux-gnuabi64',
    'gcc-8-mips64el-linux-gnuabi64-base',
    'libatomic1-mips64el-cross',
    'libc6-dev-mips64el-cross',
    'libc6-mips64el-cross',
    'libgcc-8-dev-mips64el-cross',
    'libgcc1-mips64el-cross',
    'libgomp1-mips64el-cross',
    'libisl19',
    'libmpfr6',  # This is new in buster, so build machines don't have it yet.
    'libstdc++-8-dev-mips64el-cross',
    'libstdc++6-mips64el-cross',
    'linux-libc-dev-mips64el-cross',
]

def create_asset(target_dir):
  """Create the asset."""
  # This is all a bit hacky. Rather than installing to a chroot, we just extract
  # all the packages to the target dir, then fix things up so that it can be
  # used in our recipes.
  with utils.tmp_dir():
    # Download required Debian packages.
    subprocess.check_call(['apt-get', 'download'] + PKGS)
    for f in os.listdir('.'):
      subprocess.check_call(['dpkg-deb', '--extract', f, target_dir])
  parent_dir = os.path.join(target_dir, 'usr')
  # Remove unnecessary files that cause problems with zipping (due to dangling
  # symlinks).
  os.remove(os.path.join(parent_dir,
                         'lib/gcc-cross/mips64el-linux-gnuabi64/8/libcc1.so'))
  shutil.rmtree(os.path.join(parent_dir, 'share'))
  # Remove usr/ prefix.
  for d in os.listdir(parent_dir):
    os.rename(os.path.join(parent_dir, d), os.path.join(target_dir, d))
  os.rmdir(parent_dir)
  # Remove absolute paths in GNU ld scripts.
  lib_dir = os.path.join(target_dir, 'mips64el-linux-gnuabi64/lib')
  ld_script_token = 'OUTPUT_FORMAT(elf64-tradlittlemips)'
  ld_script_files = subprocess.check_output(
    ['grep', '--recursive', '--files-with-matches',
     '--binary-files=without-match', '--fixed-strings', ld_script_token,
     lib_dir]).split()
  abs_path = '/usr/mips64el-linux-gnuabi64/lib/'
  for f in ld_script_files:
    with open(f) as script:
      contents = script.read()
    contents = contents.replace(abs_path, '')
    with open(f, 'w') as script:
      script.write(contents)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
