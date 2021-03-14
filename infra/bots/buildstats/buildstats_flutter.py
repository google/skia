# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Writes a Perf-formated json file with stats about Skia's size in flutter."""


import json
import os
import subprocess
import sys


def main():
  # This should be the stripped file from
  # out/android_release/lib.stripped/libflutter.so
  stripped_file = sys.argv[1]
  out_dir = sys.argv[2]
  keystr = sys.argv[3]
  propstr = sys.argv[4]
  bloaty_path = sys.argv[5]
  # This is the unstripped out/android_release/libflutter.so
  # The symbols in it are needed to get the compileunits data.
  symbols_file = sys.argv[6]
  config = sys.argv[7]
  total_size_bytes_key = sys.argv[8]
  lib_name = sys.argv[9]
  magic_seperator = sys.argv[10]

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

  # Human "readable" reports as an FYI.
  print magic_seperator
  print 'Report by file, then by symbol with ellided/combined templates'
  lines = subprocess.check_output([bloaty_path, stripped_file,
                                 '-d', 'compileunits,symbols', '-s', 'file',
                                 '-n', '0', '--tsv', '--demangle=short',
                                 '--debug-file=%s' % symbols_file])
  grand_total = print_skia_lines_file_symbol(lines)
  print magic_seperator
  print 'Report by file, then by symbol with full templates'
  lines = subprocess.check_output([bloaty_path, stripped_file,
                                 '-d', 'compileunits,symbols', '-s', 'file',
                                 '-n', '0', '--tsv', '--demangle=full',
                                 '--debug-file=%s' % symbols_file])
  print_skia_lines_file_symbol(lines)
  print magic_seperator

  print 'Report by symbol, then by file with ellided/combined templates'
  lines = subprocess.check_output([bloaty_path, stripped_file,
                                 '-d', 'symbols,compileunits', '-s', 'file',
                                 '-n', '0', '--tsv', '--demangle=short',
                                 '--debug-file=%s' % symbols_file])
  print_skia_lines_symbol_file(lines)
  print magic_seperator

  print 'Report by symbol, then by file with full templates'
  lines = subprocess.check_output([bloaty_path, stripped_file,
                                 '-d', 'symbols,compileunits', '-s', 'file',
                                 '-n', '0', '--tsv', '--demangle=full',
                                 '--debug-file=%s' % symbols_file])
  print_skia_lines_symbol_file(lines)
  print magic_seperator

  r = {
    # Use the default config as stats about the whole binary
    config : {
      total_size_bytes_key: grand_total
    },
  }

  results['results'][lib_name] = r

  # Make debugging easier
  print json.dumps(results, indent=2)

  with open(os.path.join(out_dir, lib_name+'.json'), 'w') as output:
    output.write(json.dumps(results, indent=2))


def bytes_or_kb(num):
  if num < 1024:
    return '%d bytes' % num
  else:
    return '%1.1f KiB' % (num / 1024.0)


def print_skia_lines_file_symbol(lines):
  lines = lines.split('\n')
  grand_total = 0
  sub_total = 0
  cur_file = ''

  for line in lines:
    # Line looks like:
    # ../../third_party/skia/src/file.cpp\tSkTSect<>::intersects()\t1224\t1348
    parts = line.split('\t')
    if len(parts) != 4:
      continue
    this_file = parts[0]
    if 'third_party/skia' not in this_file:
      continue
    symbol    = parts[1]
    if '.debug' in symbol:
      continue
    # vmsize    = parts[2] Not needed
    filesize  = int(parts[3])

    if this_file != cur_file:
      if cur_file != '':
        print '\t%-100s: %s' % ('Total File Size', bytes_or_kb(sub_total))
      sub_total = 0
      cur_file = this_file
      print this_file.replace('../../third_party/skia', 'skia')

    print '\t%-100s: %s' % (symbol, bytes_or_kb(filesize))
    sub_total += filesize
    grand_total += filesize

  print '\t%-100s: %s' % ('Total File Size', bytes_or_kb(sub_total))
  print '======================================='
  print 'Grand Total File Size: %s' % bytes_or_kb(grand_total)
  return grand_total


def print_skia_lines_symbol_file(lines):
  lines = lines.split('\n')

  for line in lines:
    # Line looks like:
    # SkTSect<>::intersects()\t../../third_party/skia/src/file.cpp\t1224\t1348
    parts = line.split('\t')
    if len(parts) != 4:
      continue
    symbol    = parts[0]
    if 'section' in symbol:
      continue
    this_file = parts[1]
    if 'third_party/skia' not in this_file:
      continue
    this_file = this_file.replace('../../third_party/skia', 'skia')
    # vmsize    = parts[2] Not needed
    filesize  = int(parts[3])

    print '%-10s: %-80s in %s' % (bytes_or_kb(filesize), symbol, this_file)


if __name__ == '__main__':
  main()
