#!/usr/bin/env python3
#
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import os
import shutil
import subprocess
import sys

FILE_DIR = os.path.dirname(os.path.abspath(__file__))
INFRA_BOTS_DIR = os.path.realpath(os.path.join(FILE_DIR, os.pardir, os.pardir))
sys.path.insert(0, INFRA_BOTS_DIR)
import utils

# https://cloud.google.com/storage/docs/gsutil_install#windows
URL = "https://storage.googleapis.com/pub/gsutil.zip"
VERSION = "5.25"

def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir():
    subprocess.run(["curl", URL, "--output", "gsutil.zip"], check=True)
    subprocess.run(["unzip", "gsutil.zip"], check=True)
    with open("./gsutil/VERSION", "r") as f:
      version = f.read().strip()
      if version != VERSION:
        raise RuntimeError("Action version %s does not match expected version %s"
                           % (version, VERSION))
    shutil.move('./gsutil', target_dir)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()

