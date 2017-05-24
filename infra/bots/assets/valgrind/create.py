#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import common
import os
import shutil
import subprocess
import urllib2
import utils


DOWNLOAD_URL = 'http://valgrind.org/downloads/valgrind-3.12.0.tar.bz2'


def create_asset(target_dir):
  """Create the asset."""
  with utils.tmp_dir() as d:
    filename = os.path.basename(DOWNLOAD_URL)
    with open(filename, 'wb') as f:
      f.write(urllib2.urlopen(DOWNLOAD_URL).read())
    subprocess.check_call(['tar', 'xvjf', filename])
    with utils.chdir(filename.rstrip('.tar.bz2')):
      subprocess.check_call(['./configure', '--prefix=%s' % d.name])
      subprocess.check_call(['make'])
      subprocess.check_call(['make', 'install'])
      os.mkdir(os.path.join(target_dir, 'bin'))
      shutil.copy(os.path.join(d.name, 'bin', 'valgrind'),
                  os.path.join(target_dir, 'bin', 'valgrind'))
      os.mkdir(os.path.join(target_dir, 'lib'))
      os.mkdir(os.path.join(target_dir, 'lib', 'valgrind'))
      for lib in ['memcheck-amd64-linux']:
        shutil.copy(os.path.join(d.name, 'lib', 'valgrind', lib),
                    os.path.join(target_dir, 'lib', 'valgrind', lib))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
