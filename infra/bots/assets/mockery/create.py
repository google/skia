#!/usr/bin/env python
#
# Copyright 2020 Google LLC.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""

import argparse
import subprocess
import os


URL = "https://github.com/vektra/mockery/releases/download/v2.4.0/mockery_2.4.0_Linux_x86_64.tar.gz"

def create_asset(target_dir):
  """Create the asset."""
  os.chdir(target_dir)
  output = subprocess.check_output(["wget", URL, "--output-document=mockery.tar.gz"])
  print(output)
  output = subprocess.check_output(["tar", "-xvf", "mockery.tar.gz"])
  print(output)
  os.remove("mockery.tar.gz")


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
