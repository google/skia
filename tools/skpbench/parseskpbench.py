#!/usr/bin/env python

# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from _benchresult import BenchResult
from argparse import ArgumentParser
from datetime import datetime
import collections
import operator
import os
import sys
import tempfile
import urllib
import urlparse
import webbrowser

__argparse = ArgumentParser(description="""

Parses output files from skpbench.py into csv.

This script can also be used to generate a Google sheet:

(1) Install the "Office Editing for Docs, Sheets & Slides" Chrome extension:
    https://chrome.google.com/webstore/detail/office-editing-for-docs-s/gbkeegbaiigmenfmjfclcdgdpimamgkj

(2) Designate Chrome os-wide as the default application for opening .csv files.

(3) Run parseskpbench.py with the --open flag.

""")

__argparse.add_argument('-r', '--result',
    choices=['accum', 'median', 'max', 'min'], default='accum',
    help="result to use for cell values")
__argparse.add_argument('-f', '--force',
    action='store_true', help='silently ignore warnings')
__argparse.add_argument('-o', '--open',
    action='store_true',
    help="generate a temp file and open it (theoretically in a web browser)")
__argparse.add_argument('-n', '--name',
    default='skpbench_%s' % datetime.now().strftime('%Y-%m-%d_%H.%M.%S.csv'),
    help="if using --open, a name for the temp file")
__argparse.add_argument('sources',
    nargs='+', help="source files with skpbench results ('-' for stdin)")

FLAGS = __argparse.parse_args()


class Parser:
  def __init__(self):
    self.configs = list() # use list to preserve the order configs appear in.
    self.rows = collections.defaultdict(dict)
    self.cols = collections.defaultdict(dict)
    self.metric = None
    self.sample_ms = None

  def parse_file(self, infile):
    for line in infile:
      match = BenchResult.match(line)
      if not match:
        continue
      if self.metric is None:
        self.metric = match.metric
      elif match.metric != self.metric:
        raise ValueError("results have mismatched metrics (%s and %s)" %
                         (self.metric, match.metric))
      if self.sample_ms is None:
        self.sample_ms = match.sample_ms
      elif not FLAGS.force and match.sample_ms != self.sample_ms:
        raise ValueError("results have mismatched sampling times. "
                         "(use --force to ignore)")
      if not match.config in self.configs:
        self.configs.append(match.config)
      self.rows[match.bench][match.config] = match.get_string(FLAGS.result)
      self.cols[match.config][match.bench] = getattr(match, FLAGS.result)

  def print_csv(self, outfile=sys.stdout):
    print('%s_%s' % (FLAGS.result, self.metric), file=outfile)

    # Write the header.
    outfile.write('bench,')
    for config in self.configs:
      outfile.write('%s,' % config)
    outfile.write('\n')

    # Write the rows.
    for bench, row in self.rows.items():
      outfile.write('%s,' % bench)
      for config in self.configs:
        if config in row:
          outfile.write('%s,' % row[config])
        elif FLAGS.force:
          outfile.write(',')
        else:
          raise ValueError("%s: missing value for %s. (use --force to ignore)" %
                           (bench, config))
      outfile.write('\n')

    # Add simple, literal averages.
    if len(self.rows) > 1:
      outfile.write('\n')
      self.__print_computed_row('MEAN',
        lambda col: reduce(operator.add, col.values()) / len(col),
        outfile=outfile)
      self.__print_computed_row('GEOMEAN',
        lambda col: reduce(operator.mul, col.values()) ** (1.0 / len(col)),
        outfile=outfile)

  def __print_computed_row(self, name, func, outfile=sys.stdout):
    outfile.write('%s,' % name)
    for config in self.configs:
      assert(len(self.cols[config]) == len(self.rows))
      outfile.write('%.4g,' % func(self.cols[config]))
    outfile.write('\n')


def main():
  parser = Parser()

  # Parse the input files.
  for src in FLAGS.sources:
    if src == '-':
      parser.parse_file(sys.stdin)
    else:
      with open(src, mode='r') as infile:
        parser.parse_file(infile)

  # Print the csv.
  if not FLAGS.open:
    parser.print_csv()
  else:
    dirname = tempfile.mkdtemp()
    basename = FLAGS.name
    if os.path.splitext(basename)[1] != '.csv':
      basename += '.csv';
    pathname = os.path.join(dirname, basename)
    with open(pathname, mode='w') as tmpfile:
      parser.print_csv(outfile=tmpfile)
    fileuri = urlparse.urljoin('file:', urllib.pathname2url(pathname))
    print('opening %s' % fileuri)
    webbrowser.open(fileuri)


if __name__ == '__main__':
  main()
