#!/usr/bin/env python

# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from _benchresult import BenchResult
from argparse import ArgumentTypeError, ArgumentParser
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

def parse_key_value_pairs(args):
  if not args:
    return dict()
  if len(args) % 2:
    raise ArgumentTypeError("uneven number of key/value arguments.")
  return {k:v for k,v in zip(args[::2], args[1::2])}

def skiaperf_result(benchresult):
  result = {x:benchresult.get_string(x) for x in ('accum', 'median')}
  result['options'] = {x:benchresult.get_string(x)
                       for x in ('clock', 'metric', 'sample_ms')}
  return result

def emit_as_json(data, outfile):
  json.dump(data, outfile, indent=4, separators=(',', ' : '), sort_keys=True)
  print('', file=outfile)

def main():
  data = parse_key_value_pairs(
    FLAGS.properties + [
    'key', parse_key_value_pairs(FLAGS.key),
    'results', defaultdict(dict)])

  for src in FLAGS.sources:
    with open(src, mode='r') as infile:
      for line in infile:
        match = BenchResult.match(line)
        if match:
          data['results'][match.bench][match.config] = skiaperf_result(match)

  if FLAGS.outfile != '-':
    with open(FLAGS.outfile, 'w+') as outfile:
      emit_as_json(data, outfile)
  else:
    emit_as_json(data, sys.stdout)

if __name__ == '__main__':
  main()
