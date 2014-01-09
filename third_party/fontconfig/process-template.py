#!/usr/bin/python

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# A simple template processing script.

import optparse
import os
import sys

parser = optparse.OptionParser()
parser.add_option('-i', '--input')
parser.add_option('-o', '--output')
parser.add_option(
    '-k', '--keyword_substitution',  action='append', nargs=2,
     metavar=('KEY', 'VALUE'), help='Changes KEY to VALUE in the template.')
parser.add_option(
    '-p', '--path_substitution', action='append', nargs=2,
    metavar=('KEY', 'PATH'),
    help='Makes PATH absolute then changes KEY to PATH in the template.')

(args, _) = parser.parse_args()

input = sys.stdin
if args.input:
  input = open(args.input, 'r')

output = sys.stdout
if args.output:
  output = open(args.output, 'w')

path_subs = None
if args.path_substitution:
  path_subs = [
      [sub[0], os.path.abspath(sub[1])] for sub in args.path_substitution
  ]

for line in input:
  if args.keyword_substitution:
    for (key, value) in args.keyword_substitution:
      line = line.replace(key, value)
  if path_subs:
    for (key, path) in path_subs:
      line = line.replace(key, path)
  output.write(line)

input.close()
output.close()
