#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset and upload it."""


import argparse
import os
import subprocess
import sys
import tempfile
import create


FILE_DIR = os.path.dirname(os.path.abspath(__file__))
ASSET = os.path.basename(FILE_DIR)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--chrome_src_path', '-c', required=True)
  parser.add_argument('--browser_executable', '-e', required=True)
  parser.add_argument('--upload_to_partner_bucket', action='store_true')
  parser.add_argument('--dry_run', action='store_true')
  parser.add_argument('--local', action='store_true')
  args = parser.parse_args()
  # Pass the flags to the creation script via environment variables, since
  # we're calling the script via `sk` and not directly.
  os.environ[create.BROWSER_EXECUTABLE_ENV_VAR] = args.browser_executable
  os.environ[create.CHROME_SRC_PATH_ENV_VAR] = args.chrome_src_path
  os.environ[create.UPLOAD_TO_PARTNER_BUCKET_ENV_VAR] = (
      '1' if args.upload_to_partner_bucket else '0')

  sk = os.path.realpath(os.path.join(
      FILE_DIR, os.pardir, os.pardir, os.pardir, os.pardir, 'bin', 'sk'))
  if os.name == 'nt':
    sk += '.exe'
  if not os.path.isfile(sk):
    raise Exception('`sk` not found at %s; maybe you need to run bin/fetch-sk?')

  # Find the Chromium revision and supply it as a tag.
  chromium_revision = subprocess.check_output(
      ['git', 'rev-parse', 'HEAD'], cwd=args.chrome_src_path).decode().rstrip()

  # Upload the asset.
  cmd = [
    sk, 'asset', 'upload',
    '--tags', 'chromium_revision:%s' % chromium_revision,
  ]
  if args.dry_run:
    cmd.append('--dry-run')
  if not args.local:
    cmd.append('--ci')
  cmd.append('skp')
  subprocess.check_call(cmd, cwd=FILE_DIR)


if __name__ == '__main__':
  main()
