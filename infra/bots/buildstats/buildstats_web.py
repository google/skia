# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Writes a Perf-formated json file with stats about the given web file."""


import json
import os
import subprocess
import sys


def main():
  input_file = sys.argv[1]
  out_dir = sys.argv[2]
  keystr = sys.argv[3]
  propstr = sys.argv[4]
  total_size_bytes_key = sys.argv[5]
  magic_seperator = sys.argv[6]

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
    total_size_bytes_key: os.path.getsize(input_file)
  }

  # Make a copy to avoid destroying the hardlinked file.
  # Swarming hardlinks in the builds from isolated cache.
  temp_file = input_file + '_tmp'
  subprocess.check_call(['cp', input_file, temp_file])
  subprocess.check_call(['gzip', temp_file])

  r['gzip_size_bytes'] = os.path.getsize(temp_file + '.gz')

  name = os.path.basename(input_file)

  print magic_seperator
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


if __name__ == '__main__':
  main()
