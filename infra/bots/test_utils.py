#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Test utilities."""


import filecmp
import os
import uuid


class FileWriter(object):
  """Write files into a given directory."""
  def __init__(self, cwd):
    self._cwd = cwd
    if not os.path.exists(self._cwd):
      os.makedirs(self._cwd)

  def mkdir(self, dname, mode=0o755):
    """Create the given directory with the given mode."""
    dname = os.path.join(self._cwd, dname)
    os.mkdir(dname)
    os.chmod(dname, mode)

  def write(self, fname, mode=0o640):
    """Write the file with the given mode and random contents."""
    fname = os.path.join(self._cwd, fname)
    with open(fname, 'w') as f:
      f.write(str(uuid.uuid4()))
    os.chmod(fname, mode)

  def remove(self, fname):
    """Remove the file."""
    fname = os.path.join(self._cwd, fname)
    if os.path.isfile(fname):
      os.remove(fname)
    else:
      os.rmdir(fname)


def compare_trees(test, a, b):
  """Compare two directory trees, assert if any differences."""
  def _cmp(prefix, dcmp):
    # Verify that the file and directory names are the same.
    test.assertEqual(len(dcmp.left_only), 0)
    test.assertEqual(len(dcmp.right_only), 0)
    test.assertEqual(len(dcmp.diff_files), 0)
    test.assertEqual(len(dcmp.funny_files), 0)

    # Verify that the files are identical.
    for f in dcmp.common_files:
      pathA = os.path.join(a, prefix, f)
      pathB = os.path.join(b, prefix, f)
      test.assertTrue(filecmp.cmp(pathA, pathB, shallow=False))
      statA = os.stat(pathA)
      statB = os.stat(pathB)
      test.assertEqual(statA.st_mode, statB.st_mode)
      with open(pathA, 'rb') as f:
        contentsA = f.read()
      with open(pathB, 'rb') as f:
        contentsB = f.read()
      test.assertEqual(contentsA, contentsB)

    # Recurse on subdirectories.
    for prefix, obj in dcmp.subdirs.items():
      _cmp(prefix, obj)

  _cmp('', filecmp.dircmp(a, b))
