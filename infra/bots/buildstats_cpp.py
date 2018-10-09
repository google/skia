# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Writes a Perf-formated json file with stats about the given cpp file."""


import json
import os
import subprocess
import sys


def main():
  input_file = sys.argv[1]
  out_dir = sys.argv[2]
  keystr = sys.argv[3]
  propstr = sys.argv[4]

  results = {
    'key': { },
    'results': { }
  }

  props = propstr.split(' ')
  for i in range(0, len(props), 2):
    results[props[i]] = props[i+1]

  keys = keystr.split(' ')
  for i in range(0, len(keys), 2):
    results['key'][keys[i]] = keys[i+1]

  r = {
    'total_size_bytes': os.path.getsize(input_file)
  }



  name = os.path.basename(input_file)

  results['results'][name] = {
    # We need this top level layer 'config'/slice
    # Other analysis methods (e.g. libskia) might have
    # slices for data on the 'code' section, etc.
    'default' : r,
  }

  # Make debugging easier
  print json.dumps(results, indent=2)

  with open(os.path.join(out_dir, name+'.json'), 'w') as output:
    output.write(json.dumps(results, indent=2))


if '__main__' == __name__:
  main()
