#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import common
from assets import asset_utils


def create_asset(target_dir):
  """Create the asset."""
  # The common case is to add one or more images to the existing set. Therefore,
  # download the previous version first.
  asset = asset_utils.Asset(common.ASSET_NAME, asset_utils.MultiStore())
  asset.download_current_version(target_dir)

  # Allow the user to modify the contents of the target dir.
  raw_input('Previous SKImage contents have been downloaded. Please make '
            'your desired changes in the following directory and press enter '
            'to continue:\n%s' % target_dir)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
