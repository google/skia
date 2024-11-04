#!/usr/bin/env python3
#
# Copyright 2024 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import os
import subprocess
import tempfile


URL='https://ftp.debian.org/debian/pool/main/p/patch/patch_2.7.6-7_amd64.deb'
SHA256 = '8c6d49b771530dbe26d7bd060582dc7d2b4eeb603a20789debc1ef4bbbc4ef67'


def create_asset(target_dir):
  """Create the asset."""
  tmp = tempfile.mkdtemp()
  target_file = os.path.join(tmp, 'patch.deb')
  subprocess.call(['wget', '--quiet', '--output-document', target_file, URL])
  output = subprocess.check_output(['sha256sum', target_file], encoding='utf-8')
  actual_hash = output.split(' ')[0]
  if actual_hash != SHA256:
    raise Exception('SHA256 does not match (%s != %s)' % (actual_hash, SHA256))
  subprocess.check_call(['dpkg-deb', '-x', target_file, tmp])
  subprocess.check_call(['cp', tmp + '/usr/bin/patch', target_dir])
  subprocess.check_call(['rm', '-rf', tmp])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()

