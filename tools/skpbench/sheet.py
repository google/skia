#!/usr/bin/env python

# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from _benchresult import BenchResult
from argparse import ArgumentParser
from collections import defaultdict, namedtuple
from datetime import datetime
import operator
import os
import sys
import tempfile
import urllib
import urlparse
import webbrowser

__argparse = ArgumentParser(description="""

Formats skpbench.py outputs as csv.

This script can also be used to generate a Google sheet:

(1) Install the "Office Editing for Docs, Sheets & Slides" Chrome extension:
    https://chrome.google.com/webstore/detail/office-editing-for-docs-s/gbkeegbaiigmenfmjfclcdgdpimamgkj

(2) Update your global OS file associations to use Chrome for .csv files.

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
  nargs='+', help="source files that contain skpbench results ('-' for stdin)")

FLAGS = __argparse.parse_args()

RESULT_QUALIFIERS = ('sample_ms', 'clock', 'metric')

class FullConfig(namedtuple('fullconfig', ('config',) + RESULT_QUALIFIERS)):
  def qualified_name(self, qualifiers=RESULT_QUALIFIERS):
    return get_qualified_name(self.config.replace(',', ' '),
                              {x:getattr(self, x) for x in qualifiers})

def get_qualified_name(name, qualifiers):
  if not qualifiers:
    return name
  else:
    args = ('%s=%s' % (k,v) for k,v in qualifiers.iteritems())
    return '%s (%s)' % (name, ' '.join(args))

class Parser:
  def __init__(self):
    self.sheet_qualifiers = {x:None for x in RESULT_QUALIFIERS}
    self.config_qualifiers = set()
    self.fullconfigs = list() # use list to preserve the order.
    self.rows = defaultdict(dict)
    self.cols = defaultdict(dict)

  def parse_file(self, infile):
    for line in infile:
      match = BenchResult.match(line)
      if not match:
        continue

      fullconfig = FullConfig(*(match.get_string(x)
                                for x in FullConfig._fields))
      if not fullconfig in self.fullconfigs:
        self.fullconfigs.append(fullconfig)

      for qualifier, value in self.sheet_qualifiers.items():
        if value is None:
          self.sheet_qualifiers[qualifier] = match.get_string(qualifier)
        elif value != match.get_string(qualifier):
          del self.sheet_qualifiers[qualifier]
          self.config_qualifiers.add(qualifier)

      self.rows[match.bench][fullconfig] = match.get_string(FLAGS.result)
      self.cols[fullconfig][match.bench] = getattr(match, FLAGS.result)

  def print_csv(self, outfile=sys.stdout):
    # Write the title.
    print(get_qualified_name(FLAGS.result, self.sheet_qualifiers), file=outfile)

    # Write the header.
    outfile.write('bench,')
    for fullconfig in self.fullconfigs:
      outfile.write('%s,' % fullconfig.qualified_name(self.config_qualifiers))
    outfile.write('\n')

    # Write the rows.
    for bench, row in self.rows.iteritems():
      outfile.write('%s,' % bench)
      for fullconfig in self.fullconfigs:
        if fullconfig in row:
          outfile.write('%s,' % row[fullconfig])
        elif FLAGS.force:
          outfile.write('NULL,')
        else:
          raise ValueError("%s: missing value for %s. (use --force to ignore)" %
                           (bench,
                            fullconfig.qualified_name(self.config_qualifiers)))
      outfile.write('\n')

    # Add simple, literal averages.
    if len(self.rows) > 1:
      outfile.write('\n')
      self._print_computed_row('MEAN',
        lambda col: reduce(operator.add, col.values()) / len(col),
        outfile=outfile)
      self._print_computed_row('GEOMEAN',
        lambda col: reduce(operator.mul, col.values()) ** (1.0 / len(col)),
        outfile=outfile)

  def _print_computed_row(self, name, func, outfile=sys.stdout):
    outfile.write('%s,' % name)
    for fullconfig in self.fullconfigs:
      if len(self.cols[fullconfig]) != len(self.rows):
        outfile.write('NULL,')
        continue
      outfile.write('%.4g,' % func(self.cols[fullconfig]))
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
