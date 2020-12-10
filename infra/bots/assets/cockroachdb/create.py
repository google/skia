#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the cockroachDB asset."""

import common # fixes python import path
import argparse
import shutil
import subprocess
import utils


URL = "https://binaries.cockroachdb.com/cockroach-v20.1.3.linux-amd64.tgz"

def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    p1 = subprocess.Popen(["curl", URL], stdout=subprocess.PIPE)
    p2 = subprocess.Popen(["tar", "-xzf" "-"], stdin=p1.stdout)
    p1.stdout.close()  # Allow p1 to receive a SIGPIPE if p2 exits.
    _,_ = p2.communicate()
    shutil.move('./cockroach-v20.1.3.linux-amd64/cockroach', target_dir)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
