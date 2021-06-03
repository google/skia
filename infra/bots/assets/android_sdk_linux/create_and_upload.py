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
  parser = argparse.ArgumentParser()
  parser.add_argument('--android_sdk_root')
  args = parser.parse_args()

  android_sdk_root = args.android_sdk_root
  if not android_sdk_root:
    android_sdk_root = (os.environ.get('ANDROID_HOME') or
                        os.environ.get('ANDROID_SDK_ROOT'))
  if not android_sdk_root:
    raise Exception('No --android_sdk_root provided and no ANDROID_HOME or '
                    'ANDROID_SDK_ROOT environment variables.')

  os.environ[create.ENV_VAR] = android_sdk_root

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
