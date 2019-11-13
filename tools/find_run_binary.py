#!/usr/bin/python
# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Module that finds and runs a binary by looking in the likely locations."""


import os
import subprocess
import sys


def run_command(args):
  """Runs a program from the command line and returns stdout.

  Args:
    args: Command line to run, as a list of string parameters. args[0] is the
          binary to run.

  Returns:
    stdout from the program, as a single string.

  Raises:
    Exception: the program exited with a nonzero return code.
  """
  proc = subprocess.Popen(args,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)
  (stdout, stderr) = proc.communicate()
  if proc.returncode is not 0:
    raise Exception('command "%s" failed: %s' % (args, stderr))
  return stdout


def find_path_to_program(program):
  """Returns path to an existing program binary.

  Args:
    program: Basename of the program to find (e.g., 'render_pictures').

  Returns:
    Absolute path to the program binary, as a string.

  Raises:
    Exception: unable to find the program binary.
  """
  trunk_path = os.path.abspath(os.path.join(os.path.dirname(__file__),
                                            os.pardir))
  possible_paths = [os.path.join(trunk_path, 'out', 'Release', program),
                    os.path.join(trunk_path, 'out', 'Debug', program),
                    os.path.join(trunk_path, 'out', 'Release',
                                 program + '.exe'),
                    os.path.join(trunk_path, 'out', 'Debug',
                                 program + '.exe')]
  for try_path in possible_paths:
    if os.path.isfile(try_path):
      return try_path
  raise Exception('cannot find %s in paths %s; maybe you need to '
                  'build %s?' % (program, possible_paths, program))

