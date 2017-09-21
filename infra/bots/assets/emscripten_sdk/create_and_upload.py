#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
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
  if 'linux' not in sys.platform:
    print >> sys.stderr, 'This script only runs on Linux.'
    sys.exit(1)
  parser = argparse.ArgumentParser()
  parser.add_argument('--gsutil')
  parser.add_argument('--sdk_path', '-s', required=True)
  args = parser.parse_args()

  with utils.tmp_dir():
    upload_script = os.path.join(common.FILE_DIR, 'upload.py')

    try:
      cmd = ['python', upload_script, '-t', args.sdk_path]
      if args.gsutil:
        cmd.extend(['--gsutil', args.gsutil])
      subprocess.check_call(cmd)
    except subprocess.CalledProcessError:
      # Trap exceptions to avoid printing two stacktraces.
      sys.exit(1)


if __name__ == '__main__':
  main()
