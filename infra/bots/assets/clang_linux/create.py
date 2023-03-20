#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create a Clang toolchain for Linux hosts."""


import argparse
import os
import subprocess


def create_asset(target_dir):
  # Create a local docker image tagged "clang_linux_asset" with a compiled clang for amd64 Linux.
  # This will take a while. Output is in the image under /tmp/clang_output
  args = ['docker', 'build', '-t', 'clang_linux_asset', './infra/bots/assets/clang_linux']
  subprocess.run(args, check=True, encoding='utf8')

  # Copy the assets out of the container by mounting target_dir
  print('Copying clang from Docker container into CIPD folder')
  os.makedirs(target_dir, exist_ok=True)
  args = ['docker', 'run', '--mount', 'type=bind,source=%s,target=/OUT' % target_dir,
          'clang_linux_asset', '/bin/sh', '-c',
          # After copying, we need to make the files write-able by all users.
          # Docker makes them owned by root by default, and without everyone being
          # able to write (e.g. delete) them, this can cause issues.
          'cp -R /tmp/clang_output/* /OUT && chmod -R a+w /OUT']
  subprocess.run(args, check=True, encoding='utf8')


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
