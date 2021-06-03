#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
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
  parser = argparse.ArgumentParser()
  parser.add_argument('--src_dir', '-s', required=True)
  args = parser.parse_args()
  # Pass src_dir to the creation script via an environment variable, since
  # we're calling the script via `sk` and not directly.
  os.environ[create.ENV_VAR] = args.src_dir

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
