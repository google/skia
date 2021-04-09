#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import common  # pylint: disable=unused-import
import os
import subprocess
import utils


# Remember to also update the go_win asset when this is updated.
GO_URL = "https://golang.org/dl/go1.16.3.linux-amd64.tar.gz"


def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    cwd = os.getcwd()
    zipfile = os.path.join(cwd, 'go.tar.gz')
    subprocess.check_call(["wget", '-O', zipfile, GO_URL])
    subprocess.check_call(["tar", "-xzf", zipfile, "-C", target_dir])

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
