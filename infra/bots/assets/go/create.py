#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import subprocess

GO_URL = "https://dl.google.com/go/go1.12.4.linux-amd64.tar.gz"

def create_asset(target_dir):
  """Create the asset."""
  p1 = subprocess.Popen(["curl", GO_URL], stdout=subprocess.PIPE)
  p2 = subprocess.Popen(["tar", "-C", target_dir, "-xzf" "-"], stdin=p1.stdout)
  p1.stdout.close()  # Allow p1 to receive a SIGPIPE if p2 exits.
  _,_ = p2.communicate()


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
