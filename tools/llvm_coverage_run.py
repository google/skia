#!/usr/bin/env python
# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Run the given command through LLVM's coverage tools."""


import argparse
import json
import os
import re
import shlex
import subprocess
import sys


BUILDTYPE = 'Coverage'
PROFILE_DATA = 'default.profraw'
PROFILE_DATA_MERGED = 'prof_merged'
SKIA_OUT = 'SKIA_OUT'


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


def _testname(filename):
  """Transform the file name into an ingestible test name."""
  return re.sub(r'[^a-zA-Z0-9]', '_', filename)


def _nanobench_json(results, properties, key):
  """Return the results in JSON format like that produced by nanobench."""
  rv = {}
  # Copy over the properties first, then set the 'key' and 'results' keys,
  # in order to avoid bad formatting in case the user passes in a properties
  # dict containing those keys.
  rv.update(properties)
  rv['key'] = key
  rv['results'] = {
    _testname(f): {
      'coverage': {
        'percent': percent,
        'options': {
          'fullname': f,
          'dir': os.path.dirname(f),
        },
      },
    } for percent, f in results
  }
  return rv


def _parse_key_value(kv_list):
  """Return a dict whose key/value pairs are derived from the given list.

  For example:

      ['k1', 'v1', 'k2', 'v2']
  becomes:

      {'k1': 'v1',
       'k2': 'v2'}
  """
  if len(kv_list) % 2 != 0:
    raise Exception('Invalid key/value pairs: %s' % kv_list)

  rv = {}
  for i in xrange(len(kv_list) / 2):
    rv[kv_list[i*2]] = kv_list[i*2+1]
  return rv


def main():
  """Run coverage and generate a report."""
  # Parse args.
  parser = argparse.ArgumentParser()
  parser.add_argument('--outResultsFile')
  parser.add_argument(
      '--key', metavar='key_or_value', nargs='+',
      help='key/value pairs identifying this bot.')
  parser.add_argument(
      '--properties', metavar='key_or_value', nargs='+',
      help='key/value pairs representing properties of this build.')
  args, cmd = parser.parse_known_args()

  # We still need to pass the args we stripped out to DM.
  cmd.append('--key')
  cmd.extend(args.key)
  cmd.append('--properties')
  cmd.extend(args.properties)

  # Parse the key and properties for use in the nanobench JSON output.
  key = _parse_key_value(args.key)
  properties = _parse_key_value(args.properties)

  # Run coverage.
  results = run_coverage(cmd)

  # Write results.
  format_results = _nanobench_json(results, properties, key)
  if args.outResultsFile:
    with open(args.outResultsFile, 'w') as f:
      json.dump(format_results, f)
  else:
    print json.dumps(format_results, indent=4, sort_keys=True)


if __name__ == '__main__':
  main()
