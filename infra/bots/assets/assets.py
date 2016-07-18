#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Tool for managing assets."""


import argparse
import asset_utils
import os
import shutil
import subprocess
import sys

FILE_DIR = os.path.dirname(os.path.abspath(__file__))
INFRA_BOTS_DIR = os.path.realpath(os.path.join(FILE_DIR, os.pardir))

sys.path.insert(0, INFRA_BOTS_DIR)
import utils


def add(args):
  """Add a new asset."""
  asset_utils.Asset.add(args.asset_name, asset_utils.MultiStore())


def remove(args):
  """Remove an asset."""
  asset_utils.Asset(args.asset_name, asset_utils.MultiStore()).remove()


def download(args):
  """Download the current version of an asset."""
  asset = asset_utils.Asset(args.asset_name, asset_utils.MultiStore())
  asset.download_current_version(args.target_dir)


def upload(args):
  """Upload a new version of the asset."""
  asset = asset_utils.Asset(args.asset_name, asset_utils.MultiStore())
  asset.upload_new_version(args.target_dir, commit=args.commit)


def main(argv):
  parser = argparse.ArgumentParser(description='Tool for managing assets.')
  subs = parser.add_subparsers(help='Commands:')

  prs_add = subs.add_parser('add', help='Add a new asset.')
  prs_add.set_defaults(func=add)
  prs_add.add_argument('asset_name', help='Name of the asset.')

  prs_remove = subs.add_parser('remove', help='Remove an asset.')
  prs_remove.set_defaults(func=remove)
  prs_remove.add_argument('asset_name', help='Name of the asset.')

  prs_download = subs.add_parser(
      'download', help='Download the current version of an asset.')
  prs_download.set_defaults(func=download)
  prs_download.add_argument('asset_name', help='Name of the asset.')
  prs_download.add_argument('--target_dir', '-t', required=True)
  prs_download.add_argument('--gsutil')

  prs_upload = subs.add_parser(
      'upload', help='Upload a new version of an asset.')
  prs_upload.set_defaults(func=upload)
  prs_upload.add_argument('asset_name', help='Name of the asset.')
  prs_upload.add_argument('--target_dir', '-t', required=True)
  prs_upload.add_argument('--gsutil')
  prs_upload.add_argument('--commit', action='store_true')

  args = parser.parse_args(argv)
  args.func(args)


if __name__ == '__main__':
  main(sys.argv[1:])
