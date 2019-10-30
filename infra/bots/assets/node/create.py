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

NODE_URL = "https://nodejs.org/dist/v8.11.2/node-v8.11.2-linux-x64.tar.xz"
NODE_EXTRACT_NAME = "node-v8.11.2-linux-x64"

def create_asset(target_dir):
  """Create the asset."""
  p1 = subprocess.Popen(["curl", NODE_URL], stdout=subprocess.PIPE)
  p2 = subprocess.Popen(["tar", "-C", target_dir, "-xJf" "-"], stdin=p1.stdout)
  p1.stdout.close()  # Allow p1 to receive a SIGPIPE if p2 exits.
  _,_ = p2.communicate()
  os.rename(
    os.path.join(target_dir, NODE_EXTRACT_NAME),
    os.path.join(target_dir, "node")
  )


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
