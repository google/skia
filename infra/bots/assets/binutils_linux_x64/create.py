#!/usr/bin/env python
#
# Copyright 2022 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import glob
import os
import shutil
import subprocess
import sys

FILE_DIR = os.path.dirname(os.path.abspath(__file__))
INFRA_BOTS_DIR = os.path.realpath(os.path.join(FILE_DIR, os.pardir, os.pardir))
sys.path.insert(0, INFRA_BOTS_DIR)
import utils

# https://packages.debian.org/buster/amd64/binutils-x86-64-linux-gnu/download
# The older version from buster has fewer dynamic library dependencies.
URL = 'https://ftp.debian.org/debian/pool/main/b/binutils/binutils-x86-64-linux-gnu_2.31.1-16_amd64.deb'
SHA256 = 'c1da1cffff8a024b5eca0a6795558d9e0ec88fbd24fe059490dc665dc5cac92f'

# https://packages.debian.org/buster/amd64/binutils-x86-64-linux-gnu/filelist
to_copy = {
  # If we need other files, we can add them to this mapping.
  'x86_64-linux-gnu-strip': 'strip',
}


def create_asset(target_dir):
  with utils.tmp_dir():
    subprocess.check_call(['wget', '--output-document=binutils.deb', '--quiet', URL])
    output = subprocess.check_output(['sha256sum', 'binutils.deb'], encoding='utf-8')
    actual_hash = output.split(' ')[0]
    if actual_hash != SHA256:
      raise Exception('SHA256 does not match (%s != %s)' % (actual_hash, SHA256))
    # A .deb file is just a re-named .ar file...
    subprocess.check_call(['ar', 'x', 'binutils.deb'])
    # with a control.tar.xz and a data.tar.xz in it. The binaries are in the data one.
    subprocess.check_call(['tar', '-xf', 'data.tar.xz'])
    for (orig, copy) in to_copy.items():
      shutil.copy(os.path.join('usr', 'bin', orig), os.path.join(target_dir, copy))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()

