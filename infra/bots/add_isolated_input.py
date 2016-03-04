#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import argparse
import json


"""Add the given hash to the includes section of the given isolated file."""


def add_isolated_hash(isolated_file, hash_str):
  with open(isolated_file) as f:
    isolated = json.load(f)
  isolated['includes'].append(hash_str)
  with open(isolated_file, 'w') as f:
    json.dump(isolated, f, sort_keys=True)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--isolated_file', required=True)
  parser.add_argument('--hash', required=True)
  args = parser.parse_args()
  add_isolated_hash(args.isolated_file, args.hash)


if __name__ == '__main__':
  main()
