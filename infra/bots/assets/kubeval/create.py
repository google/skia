#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import os
import subprocess
import tempfile


DOWNLOAD_URL = 'https://github.com/instrumenta/kubeval/releases/latest/download/kubeval-linux-amd64.tar.gz'


def create_asset(target_dir):
  """Create the asset."""
  tmp = tempfile.mkdtemp()
  subprocess.check_call(['wget', DOWNLOAD_URL], cwd=tmp)
  subprocess.check_call(
      ['tar', 'xf', os.path.join(tmp, 'kubeval-linux-amd64.tar.gz'), 'kubeval'],
      cwd=target_dir)
  subprocess.check_call(['rm', '-rf', tmp])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()

