#!/usr/bin/env python2.7
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re
import subprocess
import sys

cflags = '-std=c++11 -Os -fomit-frame-pointer -mavx2 -mfma -mf16c'

subprocess.check_call(['clang++'] + cflags.split() +
                      ['-c', 'src/splicer/SkSplicer_stages.cpp'] +
                      ['-o', 'stages.o'])

print '''/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSplicer_generated_DEFINED
#define SkSplicer_generated_DEFINED

// This file is generated semi-automatically with this command:
//   $ src/splicer/build_stages.py > src/splicer/SkSplicer_generated.h
'''

for line in subprocess.check_output(['otool', '-tvj', 'stages.o']).split('\n'):
  line = line.strip()
  if line == '' or line == 'stages.o:' or line == '(__TEXT,__text) section':
    continue

  m = re.match('_(.*):', line)
  if m:
    name = m.group(1)
    print 'static const unsigned char kSplice_' + m.group(1) + '[] = {'
    continue

  # Skip the leading 16 byte address and a tab,
  # leaving the hex and mnemonics of each instruction.
  line = line[17:]
  columns = line.split('\t')
  _hex  = columns[0].strip()
  instr = columns[1]
  args  = columns[2:]

  # We can't splice code that uses rip relative addressing.
  for arg in args:
    assert 'rip' not in arg

  # jmp done, the end of each stage (the address of done is not yet filled in)
  if _hex == 'e9 00 00 00 00':
    print '};'
    continue

  sys.stdout.write('    ')
  _bytes = _hex.split(' ')
  # This is the meat of things: copy the code to a C unsigned char array.
  for byte in _bytes:
    sys.stdout.write('0x' + byte + ',')
  # From here on we're just making the generated file readable and pretty.
  sys.stdout.write(' ' * (44 - 5*len(_bytes)))
  sys.stdout.write('// ' + instr)
  if args:
    sys.stdout.write(' ' * (13 - len(instr)))
    sys.stdout.write(' '.join(args))
  sys.stdout.write('\n')

print '''#endif//SkSplicer_generated_DEFINED'''
