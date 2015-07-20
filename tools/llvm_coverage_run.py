#!/usr/bin/env python
# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Run the given command through LLVM's coverage tools."""


import argparse
import os
import subprocess


BUILDTYPE = 'Coverage'
PROFILE_DATA = 'default.profraw'
PROFILE_DATA_MERGED = 'prof_merged'
SKIA_OUT = 'SKIA_OUT'


def _get_out_dir():
  """Determine the location for compiled binaries."""
  return os.path.join(os.environ.get(SKIA_OUT, os.path.realpath('out')),
                      BUILDTYPE)


def run_coverage(cmd):
  """Run the given command and return per-file coverage data.

  Assumes that the binary has been built using llvm_coverage_build and that
  LLVM 3.6 or newer is installed.
  """
  binary_path = os.path.join(_get_out_dir(), cmd[0])
  subprocess.call([binary_path] + cmd[1:])
  try:
    subprocess.check_call(
        ['llvm-profdata', 'merge', PROFILE_DATA,
         '-output=%s' % PROFILE_DATA_MERGED])
  finally:
    os.remove(PROFILE_DATA)
  try:
    return subprocess.check_output(['llvm-cov', 'show', '-no-colors',
                                    '-instr-profile', PROFILE_DATA_MERGED,
                                    binary_path])
  finally:
    os.remove(PROFILE_DATA_MERGED)


def main():
  """Run coverage and generate a report."""
  # Parse args.
  parser = argparse.ArgumentParser()
  parser.add_argument('--outResultsFile')
  args, cmd = parser.parse_known_args()

  # Run coverage.
  report = run_coverage(cmd)
  with open(args.outResultsFile, 'w') as f:
    f.write(report)


if __name__ == '__main__':
  main()
