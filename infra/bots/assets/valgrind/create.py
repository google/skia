#!/usr/bin/env python
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


from __future__ import print_function
import argparse
import grp
import os
import pwd
import shutil
import subprocess
import sys
import tempfile
from urllib.request import urlopen

FILE_DIR = os.path.dirname(os.path.abspath(__file__))
INFRA_BOTS_DIR = os.path.realpath(os.path.join(FILE_DIR, os.pardir, os.pardir))
sys.path.insert(0, INFRA_BOTS_DIR)
import utils


VALGRIND = 'valgrind-3.15.0'
TARBALL = '%s.tar.bz2' % VALGRIND
DOWNLOAD_URL = 'ftp://sourceware.org/pub/valgrind/%s' % TARBALL
TEMP_DIR = os.path.join(tempfile.gettempdir(), 'skia-%s' % VALGRIND)
INSTALL_DIR = os.path.join(TEMP_DIR, 'valgrind_install')


def download_tarball():
  with utils.chdir(TEMP_DIR):
    if os.path.isfile(TARBALL):
      return
    with open(TARBALL, 'wb') as f:
      f.write(urlopen(DOWNLOAD_URL).read())


def unzip_tarball():
  with utils.chdir(TEMP_DIR):
    if os.path.isdir(VALGRIND):
      return
    subprocess.check_call(['tar', 'xvjf', TARBALL])


def create_install_dir():
  if os.path.isdir(INSTALL_DIR):
    return
  os.makedirs(INSTALL_DIR)


def build_valgrind():
  if os.path.isfile(os.path.join(INSTALL_DIR, 'bin', 'valgrind')):
    return
  with utils.chdir(os.path.join(TEMP_DIR, VALGRIND)):
    subprocess.check_call(['./configure', '--prefix=%s' % INSTALL_DIR])
    subprocess.check_call(['make'])
    subprocess.check_call(['make', 'install'])


def copy_files(target_dir):
  with utils.chdir(os.path.join(TEMP_DIR, VALGRIND)):
    os.mkdir(os.path.join(target_dir, 'bin'))
    shutil.copy(os.path.join(INSTALL_DIR, 'bin', 'valgrind'),
                os.path.join(target_dir, 'bin', 'valgrind'))
    os.mkdir(os.path.join(target_dir, 'lib'))
    os.mkdir(os.path.join(target_dir, 'lib', 'valgrind'))
    for lib in ['memcheck-amd64-linux']:
      shutil.copy(os.path.join(INSTALL_DIR, 'lib', 'valgrind', lib),
                  os.path.join(target_dir, 'lib', 'valgrind', lib))
    for lib in ['core', 'memcheck']:
      libname = 'vgpreload_%s-amd64-linux.so' % lib
      shutil.copy(os.path.join(INSTALL_DIR, 'lib', 'valgrind', libname),
                  os.path.join(target_dir, 'lib', 'valgrind', libname))

    shutil.copy('default.supp',
                os.path.join(target_dir, 'lib', 'valgrind', 'default.supp'))


def create_asset(target_dir):
  """Create the asset."""
  if os.name == 'nt':
    print('This script does not run on Windows.')
    sys.exit(1)

  create_install_dir()
  if not os.path.isdir(TEMP_DIR):
    os.makedirs(TEMP_DIR)
  download_tarball()
  unzip_tarball()
  build_valgrind()
  copy_files(target_dir)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
