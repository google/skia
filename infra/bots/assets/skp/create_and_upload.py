#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset and upload it."""


import argparse
import common
import os
import subprocess
import sys
import utils


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--chrome_src_path', '-c', required=True)
  parser.add_argument('--browser_executable', '-e', required=True)
  parser.add_argument('--gsutil')
  args = parser.parse_args()

  with utils.tmp_dir():
    cwd = os.getcwd()
    create_script = os.path.join(common.FILE_DIR, 'create.py')
    upload_script = os.path.join(common.FILE_DIR, 'upload.py')

    try:
      subprocess.check_call(['python', create_script, '--target_dir', cwd,
                             '--chrome_src_path', args.chrome_src_path,
                             '--browser_executable', args.browser_executable])
      cmd = ['python', upload_script, '-t', cwd]
      if args.gsutil:
        cmd.extend(['--gsutil', args.gsutil])
      subprocess.check_call(cmd)
    except subprocess.CalledProcessError:
      # Trap exceptions to avoid printing two stacktraces.
      sys.exit(1)


if __name__ == '__main__':
  main()
