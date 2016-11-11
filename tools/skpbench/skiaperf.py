#!/usr/bin/env python

# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from _benchresult import BenchResult
from argparse import ArgumentParser
from collections import defaultdict
import json
import sys

__argparse = ArgumentParser(description="""

Formats skpbench.py outputs for Skia Perf.

""")

__argparse.add_argument('sources',
  nargs='+', help="source files that contain skpbench results")
__argparse.add_argument('--properties',
  nargs='*', default=list(),
  help="space-separated key/value pairs identifying the run")
__argparse.add_argument('--key',
  nargs='*', default=list(),
  help="space-separated key/value pairs identifying the builder")
__argparse.add_argument('-o', '--outfile',
  default='-', help="output file ('-' for stdout)")

FLAGS = __argparse.parse_args()

class JSONDict(dict):
  """Simple class for building a JSON dictionary

  Returns another JSONDict when getting an undefined item, and does not allow
  an item to change once it has been inserted.

  """
  @classmethod
  def parse_from_key_value_pairs(cls, args):
    if not args:
      return JSONDict()
    if len(args) % 2:
      raise Exception("uneven number of key/value arguments.")
    return cls((k,v) for k,v in zip(args[::2], args[1::2]))

  def __init__(self, *args):
    dict.__init__(self, *args)

  def __getitem__(self, key):
    if not key in self:
      dict.__setitem__(self, key, JSONDict())
    return dict.__getitem__(self, key)

  def __setitem__(self, key, val):
    if key in self:
      raise Exception("%s: tried to set already-defined JSONDict item" % key)
    dict.__setitem__(self, key, val)

  def emit(self, outfile):
    json.dump(self, outfile, indent=4, separators=(',', ' : '), sort_keys=True)
    print('', file=outfile)

def main():
  data = JSONDict.parse_from_key_value_pairs(
    FLAGS.properties + ['key', JSONDict.parse_from_key_value_pairs(FLAGS.key)])

  for src in FLAGS.sources:
    with open(src, mode='r') as infile:
      for line in infile:
        match = BenchResult.match(line)
        if not match:
          continue
        if match.sample_ms != 50:
          raise Exception("%s: unexpected sample_ms != 50" % match.sample_ms)
        data['results'][match.bench] \
            ['%s_%s' % (match.config, match.metric)] \
            ['%s_clock' % match.clock] = {x:match.get_string(x)
                                          for x in ('accum', 'median')}
  if FLAGS.outfile != '-':
    with open(FLAGS.outfile, 'w+') as outfile:
      data.emit(outfile)
  else:
    data.emit(sys.stdout)

if __name__ == '__main__':
  main()
