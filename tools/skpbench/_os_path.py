# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from os import path
import glob

def join(*pathnames):
  return path.join(*pathnames)

def basename(pathname):
  return pathname.basename(pathname)

def find_skps(skps):
  pathnames = list()
  for skp in skps:
    if (path.isdir(skp)):
      pathnames.extend(glob.iglob(path.join(skp, '*.skp')))
    else:
      pathnames.append(skp)
  return pathnames
