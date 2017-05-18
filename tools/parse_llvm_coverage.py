#!/usr/bin/env python
# Copyright (c) 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Parse an LLVM coverage report to generate useable results."""


import argparse
import json
import os
import re
import subprocess
import sys


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


def _file_in_repo(filename, all_files):
  """Return the name of the checked-in file matching the given filename.

  Use suffix matching to determine which checked-in files the given filename
  matches. If there are no matches or multiple matches, return None.
  """
  new_file = _fix_filename(filename)
  matched = []
  for f in all_files:
    if f.endswith(new_file):
      matched.append(f)
  if len(matched) == 1:
    return matched[0]
  elif len(matched) > 1:
    print >> sys.stderr, ('WARNING: multiple matches for %s; skipping:\n\t%s'
                          % (new_file, '\n\t'.join(matched)))
  return None


def _get_per_file_per_line_coverage(report):
  """Return a dict whose keys are file names and values are coverage data.

  Values are lists which take the form (lineno, coverage, code).
  """
  all_files = []
  for root, dirs, files in os.walk(os.getcwd()):
    if 'third_party/externals' in root:
      continue
    files = [f for f in files if not (f[0] == '.' or f.endswith('.pyc'))]
    dirs[:] = [d for d in dirs if not d[0] == '.']
    for name in files:
      all_files.append(os.path.join(root[(len(os.getcwd()) + 1):], name))
  all_files.sort()

  lines = report.splitlines()
  current_file = None
  file_lines = []
  files = {}
  not_checked_in = '%' # Use this as the file name for not-checked-in files.
  for line in lines:
    m = re.match('([a-zA-Z0-9\./_-]+):', line)
    if m:
      if current_file and current_file != not_checked_in:
        files[current_file] = file_lines
      match_filename = _file_in_repo(m.groups()[0], all_files)
      current_file = match_filename or not_checked_in
      file_lines = []
    else:
      if current_file != not_checked_in:
        skip = re.match('^\s{2}-+$|^\s{2}\|.+$', line)
        if line and not skip:
          cov, linenum, code = line.split('|', 2)
          cov = cov.strip()
          if cov:
            cov = int(cov)
          else:
            cov = None # We don't care about coverage for this line.
          linenum = int(linenum.strip())
          assert linenum == len(file_lines) + 1
          file_lines.append((linenum, cov, code.decode('utf-8', 'replace')))
  return files



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
        'lines_not_covered': not_covered_lines,
        'options': {
          'fullname': f,
          'dir': os.path.dirname(f),
          'source_type': 'coverage',
        },
      },
    } for percent, not_covered_lines, f in results
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


def _get_per_file_summaries(line_by_line):
  """Summarize the full line-by-line coverage report by file."""
  per_file = []
  for filepath, lines in line_by_line.iteritems():
    total_lines = 0
    covered_lines = 0
    for _, cov, _ in lines:
      if cov is not None:
        total_lines += 1
        if cov > 0:
          covered_lines += 1
    if total_lines > 0:
      per_file.append((float(covered_lines)/float(total_lines)*100.0,
                       total_lines - covered_lines,
                       filepath))
  return per_file


def main():
  """Generate useful data from a coverage report."""
  # Parse args.
  parser = argparse.ArgumentParser()
  parser.add_argument('--report', help='input file; an llvm coverage report.',
                      required=True)
  parser.add_argument('--nanobench', help='output file for nanobench data.')
  parser.add_argument(
      '--key', metavar='key_or_value', nargs='+',
      help='key/value pairs identifying this bot.')
  parser.add_argument(
      '--properties', metavar='key_or_value', nargs='+',
      help='key/value pairs representing properties of this build.')
  parser.add_argument('--linebyline',
                      help='output file for line-by-line JSON data.')
  args = parser.parse_args()

  if args.nanobench and not (args.key and args.properties):
    raise Exception('--key and --properties are required with --nanobench')

  with open(args.report) as f:
    report = f.read()

  line_by_line = _get_per_file_per_line_coverage(report)

  if args.linebyline:
    with open(args.linebyline, 'w') as f:
      json.dump(line_by_line, f)

  if args.nanobench:
    # Parse the key and properties for use in the nanobench JSON output.
    key = _parse_key_value(args.key)
    properties = _parse_key_value(args.properties)

    # Get per-file summaries.
    per_file = _get_per_file_summaries(line_by_line)

    # Write results.
    format_results = _nanobench_json(per_file, properties, key)
    with open(args.nanobench, 'w') as f:
      json.dump(format_results, f)


if __name__ == '__main__':
  main()
