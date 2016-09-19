# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import _adb
import re
import subprocess

__ADB_DEVICE_SERIAL = None

def set_device_serial(device_serial):
  global __ADB_DEVICE_SERIAL
  __ADB_DEVICE_SERIAL = device_serial

def join(*pathnames):
  return '/'.join(pathnames)

def basename(pathname):
  return pathname.rsplit('/', maxsplit=1)[-1]

def find_skps(skps):
  escapedskps = [re.sub(r'([^a-zA-Z0-9_\*\?\[\!\]])', r'\\\1', x) # Keep globs.
                 for x in skps]
  pathnames = _adb.check('''
    for PATHNAME in %s; do
      if [ -d "$PATHNAME" ]; then
        ls "$PATHNAME"/*.skp
      else
        echo "$PATHNAME"
      fi
    done''' % ' '.join(escapedskps), device_serial=__ADB_DEVICE_SERIAL)
  return re.split('[\r\n]+', pathnames)
