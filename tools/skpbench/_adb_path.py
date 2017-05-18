# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from _adb import Adb
import re
import subprocess

__ADB = None

def init(device_serial):
  global __ADB
  __ADB = Adb(device_serial)

def join(*pathnames):
  return '/'.join(pathnames)

def basename(pathname):
  return pathname.rsplit('/', maxsplit=1)[-1]

def find_skps(skps):
  escapedskps = [re.sub(r'([^a-zA-Z0-9_/\.\*\?\[\!\]])', r'\\\1', x)
                 for x in skps]
  return __ADB.check('''\
    for PATHNAME in %s; do
      if [ -d "$PATHNAME" ]; then
        find "$PATHNAME" -maxdepth 1 -name *.skp
      else
        echo "$PATHNAME"
      fi
    done''' % ' '.join(escapedskps)).splitlines()
