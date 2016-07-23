#!/usr/bin/env python

# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import subprocess
import sys

def quiet(*cmd):
  cmd = ' '.join(cmd).split()
  subprocess.check_output(cmd)

def loud(*cmd):
  cmd = ' '.join(cmd).split()
  ret = subprocess.call(cmd)
  if ret != 0:
    sys.exit(ret)

def gn_path():
  if 'linux' in sys.platform:
    return 'buildtools/linux64/gn'
  if 'darwin' in sys.platform:
    return 'buildtools/mac/gn'
  return 'buildtools/win/gn.exe'

# Make sure we've got an up-to-date GN and Clang, and sysroot on Linux.
quiet('download_from_google_storage',
     '--no_resume --no_auth --bucket chromium-gn',
     '-s ', gn_path() + '.sha1')
quiet('python tools/clang/scripts/update.py --if-needed')
if 'linux' in sys.platform:
  quiet('python build/linux/sysroot_scripts/install-sysroot.py --arch=amd64')

# Pass all our arguments over to the real GN binary.
loud(gn_path(), *sys.argv[1:])
