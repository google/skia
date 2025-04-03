#!/usr/bin/env python
#
# Copyright 2025 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


from __future__ import print_function
import argparse
import fileinput
import os
import shutil
import subprocess
import sys


def create_asset(target_dir):
  # Create a local docker image tagged "arm64_sysroot" with a sysroot for
  # building arm64 targets. Output is in the image under
  # /tmp/arm64_sysroot_output.
  args = ['docker', 'build', '-t', 'arm64_sysroot', './infra/bots/assets/arm64_sysroot']
  subprocess.run(args, check=True, encoding='utf8')

  # Copy the assets out of the container by mounting target_dir
  print('Copying assets from Docker container into CIPD folder')
  os.makedirs(target_dir, exist_ok=True)
  args = ['docker', 'run', '--mount', 'type=bind,source=%s,target=/OUT' % target_dir,
          'arm64_sysroot', '/bin/sh', '-c',
          # After copying, we need to make the files write-able by all users.
          # Docker makes them owned by root by default, and without everyone being
          # able to write (e.g. delete) them, this can cause issues.
          'cp -R /tmp/arm64_sysroot_output/* /OUT && chmod -R a+w /OUT']
  subprocess.run(args, check=True, encoding='utf8')

def main():
  if 'linux' not in sys.platform:
    print('This script only runs on Linux.', file=sys.stderr)
    sys.exit(1)
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
