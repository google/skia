#!/usr/bin/python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Test makefile_writer.py
"""

import argparse
import os
import shutil
import sys
import tempfile
import test_variables
import unittest
import utils

sys.path.append(test_variables.GYP_GEN_DIR)

import makefile_writer
import tool_makefile_writer
import vars_dict_lib

MAKEFILE_NAME = test_variables.ANDROID_MK
REBASELINE_MSG = ('If you\'ve modified makefile_writer.py, run '
                  '"makefile_writer_tests.py --rebaseline" to rebaseline')
TOOL_DIR = 'tool'

def generate_dummy_vars_dict(name):
  """Create a VarsDict and fill it with dummy entries.

  Args:
      name: string to be appended to each entry, if not None.

  Returns:
      A VarsDict with dummy entries.
  """
  vars_dict = vars_dict_lib.VarsDict()
  for key in vars_dict.keys():
    entry = key.lower()
    if name:
      entry += '_' + name
    vars_dict[key].add(entry)
  return vars_dict

def generate_write_local_vars_params():
  """Generator to compute params for write_local_vars tests.

  Each iteration yields a new tuple: (filename, append, name), specific to a
  way to call write_local_vars for the tests.

  Yields:
      filename: filename corresponding to the expectation file for this
          combination of params to write_local_vars.
      append: boolean to pass as append parameter to write_local_vars.
      name: string to pass as name parameter to write_local_vars.
  """
  for append in [ True, False ]:
    for name in [ None, 'arm', 'foo' ]:
      filename = 'write_local_vars'
      if append:
        filename += '_append'
      else:
        filename += '_no_append'
      if name:
        filename += '_' + name
      else:
        filename += '_no_name'

      yield (filename, append, name)

def generate_dummy_vars_dict_data(name, condition):
  """Create a dummy VarsDictData.

  Create a dummy VarsDictData, using the name for both the contained
  VarsDict and the VarsDictData

  Args:
      name: name used by both the returned VarsDictData and its contained
          VarsDict.
      condition: condition used by the returned VarsDictData.

  Returns:
      A VarsDictData with dummy values, using the passed in info.
  """
  vars_dict = generate_dummy_vars_dict(name)

  return makefile_writer.VarsDictData(vars_dict=vars_dict, name=name,
                                      condition=condition)


def generate_dummy_makefile(target_dir):
  """Create a dummy makefile to demonstrate how it works.

  Use dummy values unrelated to any gyp files. Its output should remain the
  same unless/until makefile_writer.write_android_mk changes.

  Args:
      target_dir: directory in which to write the resulting Android.mk
  """
  common_vars_dict = generate_dummy_vars_dict(None)

  deviation_params = [('foo', 'COND'), ('bar', None)]
  deviations = [generate_dummy_vars_dict_data(name, condition)
                for (name, condition) in deviation_params]

  makefile_writer.write_android_mk(target_dir=target_dir,
                                   common=common_vars_dict,
                                   deviations_from_common=deviations)

def generate_dummy_tool_makefile(target_dir):
  """Create a dummy makefile for a tool.

  Args:
      target_dir: directory in which to write the resulting Android.mk
  """
  vars_dict = generate_dummy_vars_dict(None)
  tool_makefile_writer.write_tool_android_mk(target_dir=target_dir,
                                             var_dict=vars_dict)


class MakefileWriterTest(unittest.TestCase):

  def test_write_group_empty(self):
    f = tempfile.TemporaryFile()
    assert f.tell() == 0
    for empty in (None, []):
      for truth in (True, False):
        makefile_writer.write_group(f, 'name', empty, truth)
        self.assertEqual(f.tell(), 0)
    f.close()

  def test_write_group(self):
    animals = ('dog', 'cat', 'mouse', 'elephant')
    fd, filename = tempfile.mkstemp()
    with open(filename, 'w') as f:
      makefile_writer.write_group(f, 'animals', animals, False)
    os.close(fd)
    # Now confirm that it matches expectations
    utils.compare_to_expectation(filename, 'animals.txt', self.assertTrue)

    with open(filename, 'w') as f:
      makefile_writer.write_group(f, 'animals_append', animals, True)
    # Now confirm that it matches expectations
    utils.compare_to_expectation(filename, 'animals_append.txt',
                                 self.assertTrue)
    os.remove(filename)

  def test_write_local_vars(self):
    vars_dict = generate_dummy_vars_dict(None)
    # Compare various ways of calling write_local_vars to expectations.
    for (filename, append, name) in generate_write_local_vars_params():
      fd, outfile = tempfile.mkstemp()
      with open(outfile, 'w') as f:
        makefile_writer.write_local_vars(f, vars_dict, append, name)
      os.close(fd)

      # Compare to the expected file.
      utils.compare_to_expectation(outfile, filename, self.assertTrue,
                                   REBASELINE_MSG)

      # KNOWN_TARGETS is always a key in the input VarsDict, but it should not
      # be written to the resulting file.
      # Note that this assumes none of our dummy entries is 'KNOWN_TARGETS'.
      known_targets_name = 'KNOWN_TARGETS'
      self.assertEqual(len(vars_dict[known_targets_name]), 1)

      with open(outfile, 'r') as f:
        self.assertNotIn(known_targets_name, f.read())
      os.remove(outfile)

  def test_write_android_mk(self):
    outdir = tempfile.mkdtemp()
    generate_dummy_makefile(outdir)

    utils.compare_to_expectation(os.path.join(outdir, MAKEFILE_NAME),
                                 MAKEFILE_NAME, self.assertTrue, REBASELINE_MSG)

    shutil.rmtree(outdir)

  def test_tool_writer(self):
    outdir = tempfile.mkdtemp()
    tool_dir = os.path.join(outdir, TOOL_DIR)
    os.mkdir(tool_dir)
    generate_dummy_tool_makefile(tool_dir)

    utils.compare_to_expectation(os.path.join(tool_dir, MAKEFILE_NAME),
                                 os.path.join(TOOL_DIR, MAKEFILE_NAME),
                                 self.assertTrue, REBASELINE_MSG)

def main():
  loader = unittest.TestLoader()
  suite = loader.loadTestsFromTestCase(MakefileWriterTest)
  results = unittest.TextTestRunner(verbosity=2).run(suite)
  print repr(results)
  if not results.wasSuccessful():
    raise Exception('failed one or more unittests')


def rebaseline():
  generate_dummy_makefile(utils.EXPECTATIONS_DIR)

  vars_dict = generate_dummy_vars_dict(None)
  for (filename, append, name) in generate_write_local_vars_params():
    with open(os.path.join(utils.EXPECTATIONS_DIR, filename), 'w') as f:
      makefile_writer.write_local_vars(f, vars_dict, append, name)

  generate_dummy_tool_makefile(os.path.join(utils.EXPECTATIONS_DIR, TOOL_DIR))


if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('-r', '--rebaseline', help='Rebaseline expectations.',
                      action='store_true')
  args = parser.parse_args()

  if args.rebaseline:
    rebaseline()
  else:
    main()

