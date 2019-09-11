# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from _adb import Adb
import re
import subprocess

__ADB = None

def init(device_serial, adb_binary):
  global __ADB
  __ADB = Adb(device_serial, adb_binary)

def join(*pathnames):
  return '/'.join(pathnames)

def basename(pathname):
  return pathname.rsplit('/', maxsplit=1)[-1]

def find_files(dirs, file_extension):
  # root first, in case skps reside in a protected directory
  __ADB.root()
  escapeddirs = [re.sub(r'([^a-zA-Z0-9_/\.\*\?\[\!\]])', r'\\\1', x)
                 for x in dirs]
  return __ADB.check('''\
    for PATHNAME in %s; do
      if [ -d "$PATHNAME" ]; then
        find "$PATHNAME" -maxdepth 1 -name *.%s
      else
        echo "$PATHNAME"
      fi
    done''' % (' '.join(escapeddirs).splitlines(), file_extension))
