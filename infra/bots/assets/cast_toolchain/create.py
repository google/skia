#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Clones the toolchain out of the git repo."""


import argparse
import shutil
import subprocess
import os

TOOLCHAIN_REPO = 'sso://eureka-internal/prebuilt/toolchain'
TOOLCHAIN_BRANCH = 'cros_gcc49'

def create_asset(target_dir):
  """Create the asset."""
  subprocess.check_call(['prodaccess'])
  subprocess.check_call(['git', 'clone', '--depth', '1', '-b', TOOLCHAIN_BRANCH,
                        TOOLCHAIN_REPO, target_dir])
  # delete broken symlink
  os.remove(os.path.join(target_dir, 'armv7a', 'usr', 'share', 'binutils-data',
            'armv7a-cros-linux-gnueabi','2.25.51-gold'))
  shutil.rmtree(os.path.join(target_dir, 'armv7a', 'usr', 'lib',
                             'debug', '.build-id'))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
