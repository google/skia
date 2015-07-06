#!/usr/bin/env python
# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Run the given command through LLVM's coverage tools."""


import argparse
import json
import os
import shlex
import subprocess
import sys


BUILDTYPE = 'Coverage'
OUT_DIR = os.path.realpath(os.path.join('out', BUILDTYPE))
PROFILE_DATA = 'default.profraw'
PROFILE_DATA_MERGED = 'prof_merged'


def _fix_filename(filename):
  """Return a filename which we can use to identify the file.

  The file paths printed by llvm-cov take the form:

      /path/to/repo/out/dir/../../src/filename.cpp

  And then they're truncated to 22 characters with leading ellipses:

      ...../../src/filename.cpp

  This makes it really tough to determine whether the file actually belongs in
  the Skia repo.  This function strips out the leading junk so that, if the file
  exists in the repo, the returned string matches the end of some relative path
  in the repo. This doesn't guarantee correctness, but it's about as close as
  we can get.
  """
  return filename.split('..')[-1].lstrip('./')


def _filter_results(results):
  """Filter out any results for files not in the Skia repo.

  We run through the list of checked-in files and determine whether each file
  belongs in the repo. Unfortunately, llvm-cov leaves us with fragments of the
  file paths, so we can't guarantee accuracy. See the docstring for
  _fix_filename for more details.
  """
  all_files = subprocess.check_output(['git', 'ls-files']).splitlines()
  filtered = []
  for percent, filename in results:
    new_file = _fix_filename(filename)
    matched = []
    for f in all_files:
      if f.endswith(new_file):
        matched.append(f)
    if len(matched) == 1:
      filtered.append((percent, matched[0]))
    elif len(matched) > 1:
      print >> sys.stderr, ('WARNING: multiple matches for %s; skipping:\n\t%s'
                            % (new_file, '\n\t'.join(matched)))
  print 'Filtered out %d files.' % (len(results) - len(filtered))
  return filtered


def run_coverage(cmd):
  """Run the given command and return per-file coverage data.

  Assumes that the binary has been built using llvm_coverage_build and that
  LLVM 3.6 or newer is installed.
  """
  binary_path = os.path.join(OUT_DIR, cmd[0])
  subprocess.call([binary_path] + cmd[1:])
  try:
    subprocess.check_call(
        ['llvm-profdata', 'merge', PROFILE_DATA,
         '-output=%s' % PROFILE_DATA_MERGED])
  finally:
    os.remove(PROFILE_DATA)
  try:
    report = subprocess.check_output(
        ['llvm-cov', 'report', '-instr-profile', PROFILE_DATA_MERGED,
         binary_path])
  finally:
    os.remove(PROFILE_DATA_MERGED)
  results = []
  for line in report.splitlines()[2:-2]:
    filename, _, _, cover, _, _ = shlex.split(line)
    percent = float(cover.split('%')[0])
    results.append((percent, filename))
  results = _filter_results(results)
  results.sort()
  return results


def main():
  res = run_coverage(sys.argv[1:])
  print '% Covered\tFilename'
  for percent, f in res:
    print '%f\t%s' % (percent, f)


if __name__ == '__main__':
  main()
