# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Writes a Perf-formated json file with stats about the given cpp file."""


import csv
import json
import os
import re
import subprocess
import sys


def main():
  input_file = sys.argv[1]
  out_dir = sys.argv[2]
  keystr = sys.argv[3]
  propstr = sys.argv[4]
  bloaty_path = sys.argv[5]
  total_size_bytes_key = sys.argv[6]
  magic_seperator = sys.argv[7]

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

  # Human "readable" overview as an FYI.
  print magic_seperator
  print ('Note that template instantiations are grouped together, '
         'thus the elided types.')
  print subprocess.check_output([bloaty_path, input_file,
                                 '-d', 'sections,shortsymbols', '-n', '200'])
  print ' '

  sections = subprocess.check_output([bloaty_path, input_file, '-d',
                                      'sections', '-n', '0', '--csv'])

  name = os.path.basename(input_file)

  r = {
    # Use the default config as stats about the whole binary
    'default' : {
      total_size_bytes_key: os.path.getsize(input_file)
    },
  }

  # report section by section data. Sections are like .text, .data, etc.
  for section_row in sections.strip().split('\n'):
    # Follows schema sections,vmsize,filesize
    parts = section_row.split(',')
    if len(parts) < 3 or parts[0] == 'sections':
      # If we see section, that's the table header
      continue
    section = parts[0]
    # part[1] is "VM Size", part[2] is "File Size". From the bloaty docs:
    # The "VM SIZE" column tells you how much space the binary will take
    # when it is loaded into memory. The "FILE SIZE" column tells you about
    # how much space the binary is taking on disk.
    vmsize = parts[1]   # In bytes
    filesize = parts[2] # In bytes
    section = re.sub('[^0-9a-zA-Z_]', '_', section)
    r['section'+section] = {
      'in_file_size_bytes': int(filesize),
      'vm_size_bytes': int(vmsize),
    }

  print magic_seperator
  results['results'][name] = r

  # Make debugging easier
  print json.dumps(results, indent=2)

  with open(os.path.join(out_dir, name+'.json'), 'w') as output:
    output.write(json.dumps(results, indent=2))


if __name__ == '__main__':
  main()
