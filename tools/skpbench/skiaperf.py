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
  nargs='*', help="space-separated key/value pairs identifying the run")
__argparse.add_argument('--key',
  nargs='*', help="space-separated key/value pairs identifying the builder")
__argparse.add_argument('-o', '--outfile',
  default='-', help="output file ('-' for stdout)")

FLAGS = __argparse.parse_args()


class JSONDict(dict):
  """Simple class for building a JSON dictionary

  Returns another JSONDict upon accessing an undefined item. Does not allow an
  item to change once it has been inserted.

  """
  def __init__(self, key_value_pairs=None):
    dict.__init__(self)
    if not key_value_pairs:
      return
    if len(key_value_pairs) % 2:
      raise Exception("uneven number of key/value arguments.")
    for k,v in zip(key_value_pairs[::2], key_value_pairs[1::2]):
      self[k] = v

  def __getitem__(self, key):
    if not key in self:
      dict.__setitem__(self, key, JSONDict())
    return dict.__getitem__(self, key)

  def __setitem__(self, key, val):
    if key in self:
      raise Exception("%s: tried to set already-defined JSONDict item\n"
                      "  old value: '%s'\n"
                      "  new value: '%s'" % (key, self[key], val))
    dict.__setitem__(self, key, val)

  def emit(self, outfile):
    json.dump(self, outfile, indent=4, separators=(',', ' : '), sort_keys=True)
    print('', file=outfile)

def main():
  data = JSONDict(
    FLAGS.properties + \
    ['key', JSONDict(FLAGS.key + \
                     ['bench_type', 'playback', \
                      'source_type', 'skp'])])

  for src in FLAGS.sources:
    with open(src, mode='r') as infile:
      for line in infile:
        match = BenchResult.match(line)
        if not match:
          continue
        if match.sample_ms != 50:
          raise Exception("%s: unexpected sample_ms != 50" % match.sample_ms)
        for result in ('accum', 'median', 'min', 'max'):
          data['results'][match.bench][match.config] \
              ['%s_%s_%s' % (result, match.clock, match.metric)] = \
              getattr(match, result)

  if FLAGS.outfile != '-':
    with open(FLAGS.outfile, 'w+') as outfile:
      data.emit(outfile)
  else:
    data.emit(sys.stdout)

if __name__ == '__main__':
  main()
