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

# See https://cloud.google.com/sdk/downloads#versioned for documentation on
# scripting gcloud and also for updates.
BASE_URL = 'https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/%s'
GCLOUD_BASE_NAME='google-cloud-sdk'
GCLOUD_ARCHIVE = '%s-191.0.0-linux-x86_64.tar.gz' % GCLOUD_BASE_NAME
GCLOUD_URL = BASE_URL % GCLOUD_ARCHIVE

def create_asset(target_dir):
  """Create the asset."""
  subprocess.check_call(['curl', GCLOUD_URL, '-o', GCLOUD_ARCHIVE])

  # Extract the arcive to the target directory and remove it.
  subprocess.check_call(['tar', '-xzf', GCLOUD_ARCHIVE,
                         '--strip-components=1',
                         '-C', target_dir])
  subprocess.check_call(['rm', GCLOUD_ARCHIVE])

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)

if __name__ == '__main__':
  main()
