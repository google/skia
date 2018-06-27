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
import shutil
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

  upload_script = os.path.join(common.FILE_DIR, 'upload.py')

  try:
    # Remove large directories that we don't need to use the sdk
    # (e.g. intermediate build products)
    rmpath = os.path.join(args.sdk_path, 'clang', 'fastcomp',
                          'build_incoming_64')
    # We can ignore errors, for example, if the folders were already deleted.
    shutil.rmtree(os.path.join(rmpath, 'lib'), ignore_errors=True)
    shutil.rmtree(os.path.join(rmpath, 'tools') , ignore_errors=True)

    # Remove the source code, which has lots of small files which slows
    # extraction.  We don't need the source code - we mostly care about the
    # binaries in $SDK/clang/fastcomp/build_incoming_64/bin.
    src = os.path.join(args.sdk_path, 'clang', 'fastcomp', 'src')
    for name in os.listdir(src):
      p = os.path.join(src, name)
      # Purposely don't delete src/emscripten-version.txt, which can cause
      # compilation warnings about "can't verify version".
      if os.path.isdir(p):
        shutil.rmtree(p)

    cmd = ['python', upload_script, '-t', args.sdk_path]
    if args.gsutil:
      cmd.extend(['--gsutil', args.gsutil])
    subprocess.check_call(cmd)
  except subprocess.CalledProcessError:
    # Trap exceptions to avoid printing two stacktraces.
    sys.exit(1)


if __name__ == '__main__':
  main()
