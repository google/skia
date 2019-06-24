#!/usr/bin/env python
#
# Copyright 2019 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import common  # pylint: disable=unused-import
import os
import subprocess
import utils


# Remember to also update the go asset when this is updated.
GO_URL = "https://dl.google.com/go/go1.12.4.windows-amd64.zip"


def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    cwd = os.getcwd()
    zipfile = os.path.join(cwd, 'go.zip')
    subprocess.check_call(["wget", '-O', zipfile, GO_URL])
    subprocess.check_call(["unzip", zipfile, "-d", target_dir])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
