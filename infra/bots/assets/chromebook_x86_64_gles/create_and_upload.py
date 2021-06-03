#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset and upload it."""


import argparse
import os
import subprocess
import sys
import tempfile
import create


FILE_DIR = os.path.dirname(os.path.abspath(__file__))
ASSET = os.path.basename(FILE_DIR)


def main():
  if 'linux' not in sys.platform:
    print >> sys.stderr, 'This script only runs on Linux.'
    sys.exit(1)
  parser = argparse.ArgumentParser()
  parser.add_argument('--lib_path', '-l', required=True)
  args = parser.parse_args()
  # Pass lib_path to the creation script via an environment variable, since
  # we're calling the script via `sk` and not directly.
  os.environ[create.ENV_VAR] = args.lib_path

  sk = os.path.realpath(os.path.join(
      FILE_DIR, os.pardir, os.pardir, os.pardir, os.pardir, 'bin', 'sk'))
  if os.name == 'nt':
    sk += '.exe'
  if not os.path.isfile(sk):
    raise Exception('`sk` not found at %s; maybe you need to run bin/fetch-sk?')

  # Upload the asset.
  subprocess.check_call([sk, 'asset', 'upload', ASSET], cwd=FILE_DIR)


if __name__ == '__main__':
  main()
