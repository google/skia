#!/usr/bin/env python3
#
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import os
import subprocess


URL = 'https://github.com/bazelbuild/bazelisk/releases/download/v1.27.0/bazelisk-darwin-amd64'
SHA256 = '8fcd7ba828f673ba4b1529425e01e15ac42599ef566c17f320d8cbfe7b96a167'

BINARY = URL.split('/')[-1]


def create_asset(target_dir):
  """Create the asset."""
  target_file = os.path.join(target_dir, 'bazelisk')
  subprocess.call(['wget', '--quiet', '--output-document', target_file, URL])
  output = subprocess.check_output(['sha256sum', target_file], encoding='utf-8')
  actual_hash = output.split(' ')[0]
  if actual_hash != SHA256:
    raise Exception('SHA256 does not match (%s != %s)' % (actual_hash, SHA256))
  subprocess.call(['chmod', 'ugo+x', target_file])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()

