#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset and upload it."""


import os
import subprocess
import sys
import tempfile


FILE_DIR = os.path.dirname(os.path.abspath(__file__))
ASSET = os.path.basename(FILE_DIR)


def main():
  sk = os.path.realpath(os.path.join(
      FILE_DIR, os.pardir, os.pardir, os.pardir, os.pardir, 'bin', 'sk'))
  if os.name == 'nt':
    sk += '.exe'
  if not os.path.isfile(sk):
    raise Exception('`sk` not found at %s; maybe you need to run bin/fetch-sk?')

  # CIPD is picky about where files are downloaded. Use a subdirectory of the
  # asset dir rather than /tmp.
  tmp_prefix = os.path.join(FILE_DIR, '.')
  with tempfile.TemporaryDirectory(prefix=tmp_prefix) as tmp:
    subprocess.check_call([sk, 'asset', 'download', ASSET, tmp], cwd=FILE_DIR)
    # Allow the user to modify the contents of the target dir.
    input('Previous SKImage contents have been downloaded. Please make '
          'your desired changes in the following directory and press enter '
          'to continue:\n%s\n' % tmp)
    subprocess.check_call([sk, 'asset', 'upload', '--in', tmp, ASSET],
                          cwd=FILE_DIR)


if __name__ == '__main__':
  main()
