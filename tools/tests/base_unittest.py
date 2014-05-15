#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

A wrapper around the standard Python unittest library, adding features we need
for various unittests within this directory.
"""

import errno
import os
import shutil
import sys
import unittest

# Set the PYTHONPATH to include the tools directory.
sys.path.append(
    os.path.join(os.path.dirname(os.path.realpath(__file__)), os.pardir))
import find_run_binary


class TestCase(unittest.TestCase):

  def shortDescription(self):
    """Tell unittest framework to not print docstrings for test cases."""
    return None

  def create_empty_dir(self, path):
    """Creates an empty directory at path and returns path.

    Args:
      path: path on local disk
    """
    shutil.rmtree(path=path, ignore_errors=True)
    try:
      os.makedirs(path)
    except OSError as exc:
      if exc.errno != errno.EEXIST:
        raise
    return path

  def run_command(self, args):
    """Runs a program from the command line and returns stdout.

    Args:
      args: Command line to run, as a list of string parameters. args[0] is the
            binary to run.

    Returns:
      stdout from the program, as a single string.

    Raises:
      Exception: the program exited with a nonzero return code.
    """
    return find_run_binary.run_command(args)

  def find_path_to_program(self, program):
    """Returns path to an existing program binary.

    Args:
      program: Basename of the program to find (e.g., 'render_pictures').

    Returns:
      Absolute path to the program binary, as a string.

    Raises:
      Exception: unable to find the program binary.
    """
    return find_run_binary.find_path_to_program(program)


def main(test_case_class):
  """Run the unit tests within the given class.

  Raises an Exception if any of those tests fail (in case we are running in the
  context of run_all.py, which depends on that Exception to signal failures).

  TODO(epoger): Make all of our unit tests use the Python unittest framework,
  so we can leverage its ability to run *all* the tests and report failures at
  the end.
  """
  suite = unittest.TestLoader().loadTestsFromTestCase(test_case_class)
  results = unittest.TextTestRunner(verbosity=2).run(suite)
  if not results.wasSuccessful():
    raise Exception('failed unittest %s' % test_case_class)
