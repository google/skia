#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Tests for asset_utils."""


import asset_utils
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
import uuid


FILE_DIR = os.path.dirname(os.path.abspath(__file__))
INFRA_BOTS_DIR = os.path.realpath(os.path.join(
    FILE_DIR, os.pardir, 'infra', 'bots'))
sys.path.insert(0, INFRA_BOTS_DIR)
import test_utils
import utils


GS_BUCKET = 'skia-infra-testdata'


def _fake_prompt(result):
  """Make a function that pretends to prompt for input and returns a result."""
  return lambda s: result


def _write_stuff(target_dir):
  """Write some files and directories into target_dir."""
  fw = test_utils.FileWriter(target_dir)
  fw.mkdir('mydir')
  fw.mkdir('anotherdir', 0666)
  fw.mkdir('dir3', 0600)
  fw.mkdir('subdir')
  fw.write('a.txt', 0777)
  fw.write('b.txt', 0751)
  fw.write('c.txt', 0640)
  fw.write(os.path.join('subdir', 'd.txt'), 0640)


class AssetUtilsTest(unittest.TestCase):
  def setUp(self):
    self.asset_name = str(uuid.uuid4())
    self.old_prompt = asset_utils._prompt
    asset_utils._prompt = _fake_prompt('y')
    self.a = asset_utils.Asset.add(self.asset_name, gs_bucket=GS_BUCKET)

  def tearDown(self):
    if self.a:
      self.a.remove()
    asset_utils._prompt = self.old_prompt

    gs_path = 'gs://%s/assets/%s' % (GS_BUCKET, self.asset_name)
    attempt_delete = True
    try:
      subprocess.check_call(['gsutil', 'ls', gs_path])
    except subprocess.CalledProcessError:
      attempt_delete = False
    if attempt_delete:
      subprocess.check_call(['gsutil', 'rm', '-rf', gs_path])

  def test_add_remove(self):
    # Ensure that we can't create an asset twice.
    with self.assertRaises(Exception):
      asset_utils.Asset.add(self.asset_name, gs_bucket=GS_BUCKET)

    # Ensure that the asset dir exists.
    asset_dir = os.path.join(FILE_DIR, self.asset_name)
    self.assertTrue(os.path.isdir(asset_dir))

    # Remove the asset, ensure that it's gone.
    self.a.remove()
    self.a = None
    self.assertFalse(os.path.exists(asset_dir))

  def test_upload_download(self):
    with utils.tmp_dir():
      # Create input files and directories.
      input_dir = os.path.join(os.getcwd(), 'input')
      _write_stuff(input_dir)

      # Upload a version, download it again.
      self.a.upload_new_version(input_dir)
      output_dir = os.path.join(os.getcwd(), 'output')
      self.a.download_current_version(output_dir)

      # Compare.
      test_utils.compare_trees(self, input_dir, output_dir)

  def test_versions(self):
    with utils.tmp_dir():
      # Create input files and directories.
      input_dir = os.path.join(os.getcwd(), 'input')
      _write_stuff(input_dir)

      self.assertEqual(self.a.get_current_version(), -1)
      self.assertEqual(self.a.get_available_versions(), [])
      self.assertEqual(self.a.get_next_version(), 0)

      self.a.upload_new_version(input_dir)

      self.assertEqual(self.a.get_current_version(), 0)
      self.assertEqual(self.a.get_available_versions(), [0])
      self.assertEqual(self.a.get_next_version(), 1)

      self.a.upload_new_version(input_dir)

      self.assertEqual(self.a.get_current_version(), 1)
      self.assertEqual(self.a.get_available_versions(), [0, 1])
      self.assertEqual(self.a.get_next_version(), 2)


if __name__ == '__main__':
  unittest.main()
