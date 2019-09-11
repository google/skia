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

def find_files(dirs, file_extention):
  pathnames = list()
  for d in dirs:
    if (path.isdir(d)):
      pathnames.extend(glob.iglob(path.join(d, '*.'+file_extention)))
    else:
      pathnames.append(d)
  return pathnames
