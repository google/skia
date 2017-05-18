#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Utilities for zipping and unzipping files."""


import fnmatch
import os
import zipfile


def filtered(names, blacklist):
  """Filter the list of file or directory names."""
  rv = names[:]
  for pattern in blacklist:
    rv = [n for n in rv if not fnmatch.fnmatch(n, pattern)]
  return rv


def zip(target_dir, zip_file, blacklist=None):  # pylint: disable=W0622
  """Zip the given directory, write to the given zip file."""
  if not os.path.isdir(target_dir):
    raise IOError('%s does not exist!' % target_dir)
  blacklist = blacklist or []
  with zipfile.ZipFile(zip_file, 'w', zipfile.ZIP_DEFLATED, True) as z:
    for r, d, f in os.walk(target_dir, topdown=True):
      d[:] = filtered(d, blacklist)
      for filename in filtered(f, blacklist):
        filepath = os.path.join(r, filename)
        zi = zipfile.ZipInfo(filepath)
        zi.filename = os.path.relpath(filepath, target_dir)
        perms = os.stat(filepath).st_mode
        zi.external_attr = perms << 16L
        zi.compress_type = zipfile.ZIP_DEFLATED
        with open(filepath, 'rb') as f:
          content = f.read()
        z.writestr(zi, content)
      for dirname in d:
        dirpath = os.path.join(r, dirname)
        z.write(dirpath, os.path.relpath(dirpath, target_dir))


def unzip(zip_file, target_dir):
  """Unzip the given zip file into the target dir."""
  if not os.path.isdir(target_dir):
    os.makedirs(target_dir)
  with zipfile.ZipFile(zip_file, 'r', zipfile.ZIP_DEFLATED, True) as z:
    for zi in z.infolist():
      dst_path = os.path.join(target_dir, zi.filename)
      if zi.filename.endswith('/'):
        os.mkdir(dst_path)
      else:
        with open(dst_path, 'wb') as f:
          f.write(z.read(zi))
      perms = zi.external_attr >> 16L
      os.chmod(dst_path, perms)
