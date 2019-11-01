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


def _common_args(prs):
  """Add common args to the given argparse.ArgumentParser."""
  prs.add_argument('asset_name', help='Name of the asset.')
  prs.add_argument('--gsutil')
  prs.add_argument('--service_account_json')


def _store(args):
  """Return asset_utils.MultiStore based on args."""
  return asset_utils.MultiStore(gsutil=args.gsutil,
                                service_account_json=args.service_account_json)


def _asset(args):
  """Return asset_utils.Asset based on args."""
  return asset_utils.Asset(args.asset_name, _store(args))


def add(args):
  """Add a new asset."""
  asset_utils.Asset.add(args.asset_name, _store(args))


def remove(args):
  """Remove an asset."""
  _asset(args).remove()


def download(args):
  """Download the current version of an asset."""
  _asset(args).download_current_version(args.target_dir)


def upload(args):
  """Upload a new version of the asset."""
  _asset(args).upload_new_version(args.target_dir, commit=args.commit,
                                  extra_tags=args.extra_tags)


def main(argv):
  parser = argparse.ArgumentParser(description='Tool for managing assets.')
  subs = parser.add_subparsers(help='Commands:')

  prs_add = subs.add_parser('add', help='Add a new asset.')
  prs_add.set_defaults(func=add)
  _common_args(prs_add)

  prs_remove = subs.add_parser('remove', help='Remove an asset.')
  prs_remove.set_defaults(func=remove)
  _common_args(prs_remove)

  prs_download = subs.add_parser(
      'download', help='Download the current version of an asset.')
  prs_download.set_defaults(func=download)
  _common_args(prs_download)
  prs_download.add_argument('--target_dir', '-t', required=True)

  prs_upload = subs.add_parser(
      'upload', help='Upload a new version of an asset.')
  prs_upload.set_defaults(func=upload)
  _common_args(prs_upload)
  prs_upload.add_argument('--target_dir', '-t', required=True)
  prs_upload.add_argument('--commit', action='store_true')
  prs_upload.add_argument(
    '--extra_tags', nargs='+',
    help='Additional tags for the CIPD package, "key:value"')

  args = parser.parse_args(argv)
  args.func(args)


if __name__ == '__main__':
  main(sys.argv[1:])
