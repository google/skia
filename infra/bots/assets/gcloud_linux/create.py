#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import glob
import os
import shutil
import subprocess

BASE_URL = "https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/%s"
GCLOUD_BASE_NAME="google-cloud-sdk"
GCLOUD_ARCHIVE = "%s-189.0.0-linux-x86_64.tar.gz" % GCLOUD_BASE_NAME
GCLOUD_URL = BASE_URL % GCLOUD_ARCHIVE

def create_asset(target_dir):
  """Create the asset."""
  subprocess.check_call(["curl", GCLOUD_URL, "-o", GCLOUD_ARCHIVE])

  # create the GCLOUD_BASE_NAME directory and copy everything to the target dir
  subprocess.check_call(["tar", "-xzf", GCLOUD_ARCHIVE, "-C", target_dir])
  output_dir = os.path.join(target_dir, GCLOUD_BASE_NAME)
  for f in glob.glob(os.path.join(output_dir, "*")):
    shutil.move(f, target_dir)
  subprocess.check_call(["rm", GCLOUD_ARCHIVE])
  shutil.rmtree(output_dir)

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)

if __name__ == '__main__':
  main()
