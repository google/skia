#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset and upload it."""


import argparse
import common
import shutil
import os
import subprocess
import sys
import utils


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--gsutil')
  args = parser.parse_args()

  with utils.tmp_dir():
    cwd = os.getcwd()
    workdir = os.path.join(cwd, "workdir")
    create_script = os.path.join(common.FILE_DIR, 'create.py')
    upload_script = os.path.join(common.FILE_DIR, 'upload.py')

    try:
      os.mkdir(workdir)
      subprocess.check_call(['python', create_script, '-t', workdir])
      cmd = ['python', upload_script, '-t', workdir]
      if args.gsutil:
        cmd.extend(['--gsutil', args.gsutil])
      subprocess.check_call(cmd)
    except subprocess.CalledProcessError:
      # Trap exceptions to avoid printing two stacktraces.
      sys.exit(1)

if __name__ == '__main__':
  main()
