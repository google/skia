#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Tests for zip_utils."""


import filecmp
import os
import test_utils
import unittest
import utils
import uuid
import zip_utils


class ZipUtilsTest(unittest.TestCase):
  def test_zip_unzip(self):
    with utils.tmp_dir():
      fw = test_utils.FileWriter(os.path.join(os.getcwd(), 'input'))
      # Create input files and directories.
      fw.mkdir('mydir')
      fw.mkdir('anotherdir', 0o666)
      fw.mkdir('dir3', 0o600)
      fw.mkdir('subdir')
      fw.write('a.txt', 0o777)
      fw.write('b.txt', 0o751)
      fw.write('c.txt', 0o640)
      fw.write(os.path.join('subdir', 'd.txt'), 0o640)

      # Zip, unzip.
      zip_utils.zip('input', 'test.zip')
      zip_utils.unzip('test.zip', 'output')

      # Compare the inputs and outputs.
      test_utils.compare_trees(self, 'input', 'output')

  def test_to_skip(self):
    with utils.tmp_dir():
      # Create input files and directories.
      fw = test_utils.FileWriter(os.path.join(os.getcwd(), 'input'))
      fw.mkdir('.git')
      fw.write(os.path.join('.git', 'index'))
      fw.write('somefile')
      fw.write('.DS_STORE')
      fw.write('leftover.pyc')
      fw.write('.pycfile')

      # Zip, unzip.
      zip_utils.zip('input', 'test.zip', to_skip=['.git', '.DS*', '*.pyc'])
      zip_utils.unzip('test.zip', 'output')

      # Remove the files/dirs we don't expect to see in output, so that we can
      # use self._compare_trees to check the results.
      fw.remove(os.path.join('.git', 'index'))
      fw.remove('.git')
      fw.remove('.DS_STORE')
      fw.remove('leftover.pyc')

      # Compare results.
      test_utils.compare_trees(self, 'input', 'output')

  def test_nonexistent_dir(self):
    with utils.tmp_dir():
      with self.assertRaises(IOError):
        zip_utils.zip('input', 'test.zip')


if __name__ == '__main__':
  unittest.main()
