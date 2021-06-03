#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


from __future__ import print_function
import argparse
import glob
import os
import shutil
import subprocess
import sys


ENV_VAR = 'CHROMEBOOK_ARM_GLES_LIB_PATH'


def getenv(key):
  val = os.environ.get(key)
  if not val:
    print(('Environment variable %s not set; you should run this via '
           'create_and_upload.py.' % key), file=sys.stderr)
    sys.exit(1)
  return val


def create_asset(target_dir, gl_path):
  """Create the asset."""

  cmd = [
    'sudo','apt-get','install',
    'libgles2-mesa-dev',
    'libegl1-mesa-dev'
  ]
  subprocess.check_call(cmd)


  lib_dir = os.path.join(target_dir, 'lib')
  os.mkdir(lib_dir)

  to_copy = glob.glob(os.path.join(gl_path,'libGL*'))
  to_copy.extend(glob.glob(os.path.join(gl_path,'libEGL*')))
  to_copy.extend(glob.glob(os.path.join(gl_path,'libmali*')))
  for f in to_copy:
    shutil.copy(f, lib_dir)

  include_dir = os.path.join(target_dir, 'include')
  os.mkdir(include_dir)
  shutil.copytree('/usr/include/EGL', os.path.join(include_dir, 'EGL'))
  shutil.copytree('/usr/include/KHR', os.path.join(include_dir, 'KHR'))
  shutil.copytree('/usr/include/GLES2', os.path.join(include_dir, 'GLES2'))
  shutil.copytree('/usr/include/GLES3', os.path.join(include_dir, 'GLES3'))


def main():
  if 'linux' not in sys.platform:
    print('This script only runs on Linux.', file=sys.stderr)
    sys.exit(1)
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()

  # Obtain lib_path from create_and_upload via an environment variable, since
  # this script is called via `sk` and not directly.
  lib_path = getenv(ENV_VAR)

  create_asset(args.target_dir, lib_path)


if __name__ == '__main__':
  main()
