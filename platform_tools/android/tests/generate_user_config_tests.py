#!/usr/bin/python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
Test generate_user_config.py.
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

from generate_user_config import generate_user_config as gen_config

# Name of SkUserConfig file.
USER_CONFIG_NAME = 'SkUserConfig-h.txt'
MISSING_FILENAME = 'missing-filename.xxx'
# Path to unchanging Dummy SkUserConfig file.
FULL_DUMMY_PATH = os.path.join(os.path.dirname(__file__), 'inputs',
                               USER_CONFIG_NAME)
REBASELINE_MSG = ('If you\'ve modified generate_user_config.py, run '
                  '"generate_user_config_tests.py --rebaseline" to rebaseline')

def generate_dummy_user_config(original_sk_user_config,
                               require_sk_user_config, target_dir):
  # Add an arbitrary set of defines
  defines = [ 'SK_BUILD_FOR_ANDROID',
              'SK_BUILD_FOR_ANDROID_FRAMEWORK',
              'SK_SCALAR_IS_FLOAT',
              'foo',
              'bar' ]
  gen_config(original_sk_user_config=original_sk_user_config,
             require_sk_user_config=require_sk_user_config,
             target_dir=target_dir, defines=defines)


class GenUserConfigTest(unittest.TestCase):

  def test_missing_sk_user_config(self):
    tmp = tempfile.mkdtemp()
    original = os.path.join(tmp, MISSING_FILENAME)
    assert not os.path.exists(original)


    # With require_sk_user_config set to True, an AssertionError will be
    # thrown when original_sk_user_config is missing.
    with self.assertRaises(AssertionError):
      defines = [ 'define' ]
      gen_config(original_sk_user_config=original,
                 require_sk_user_config=True,
                 target_dir=tmp, defines=defines)

    # With require_sk_user_config set to False, it is okay for
    # original_sk_user_config to be missing.
    generate_dummy_user_config(original_sk_user_config=original,
                               require_sk_user_config=False, target_dir=tmp)
    actual_name = os.path.join(tmp, MISSING_FILENAME)
    utils.compare_to_expectation(actual_name=actual_name,
                                 expectation_name=MISSING_FILENAME,
                                 assert_true=self.assertTrue,
                                 msg=REBASELINE_MSG)

    shutil.rmtree(tmp)

  def test_gen_config(self):
    tmp = tempfile.mkdtemp()
    generate_dummy_user_config(FULL_DUMMY_PATH, True, tmp)
    actual_name = os.path.join(tmp, USER_CONFIG_NAME)
    utils.compare_to_expectation(actual_name=actual_name,
                        expectation_name=USER_CONFIG_NAME,
                        assert_true=self.assertTrue, msg=REBASELINE_MSG)
    shutil.rmtree(tmp)


def main():
  loader = unittest.TestLoader()
  suite = loader.loadTestsFromTestCase(GenUserConfigTest)
  results = unittest.TextTestRunner(verbosity=2).run(suite)
  print repr(results)
  if not results.wasSuccessful():
    raise Exception('failed one or more unittests')


def rebaseline():
  generate_dummy_user_config(FULL_DUMMY_PATH, True, utils.EXPECTATIONS_DIR)
  generate_dummy_user_config(MISSING_FILENAME, False, utils.EXPECTATIONS_DIR)

if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument('-r', '--rebaseline', help='Rebaseline expectations.',
                      action='store_true')
  args = parser.parse_args()

  if args.rebaseline:
    rebaseline()
  else:
    main()

