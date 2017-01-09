#!/usr/bin/env python2.7
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re
import subprocess
import sys

cflags = '-std=c++11 -Os -fomit-frame-pointer'.split()

hsw = '-mavx2 -mfma -mf16c'.split()
subprocess.check_call(['clang++'] + cflags + hsw +
                      ['-c', 'src/splicer/SkSplicer_stages.cpp'] +
                      ['-o', 'hsw.o'])

aarch64 = [
    '--target=aarch64-linux-android',
    '--sysroot=' +
    '/Users/mtklein/brew/opt/android-ndk/platforms/android-21/arch-arm64',
]
subprocess.check_call(['clang++'] + cflags + aarch64 +
                      ['-c', 'src/splicer/SkSplicer_stages.cpp'] +
                      ['-o', 'aarch64.o'])

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

#if defined(__aarch64__)
'''
for line in subprocess.check_output(['gobjdump', '-d',
                                     'aarch64.o']).split('\n'):
  line = line.strip()
  if not line or line.startswith('aarch64.o') or line.startswith('Disassembly'):
    continue

  m = re.match('''................ <(.*)>:''', line)
  if m:
    print 'static const unsigned int kSplice_' + m.group(1) + '[] = {'
    continue

  _, code, inst, args = line.split('\t')
  code = code.strip()

  # b done, where done has not yet been filled in by the linker.
  if code == '14000000':
    print '};'
    continue
  print '   ', '0x'+code+',' + '    // ' + inst + ' ' + args

print '''
#else
'''

# TODO: port this to gobjdump too
for line in subprocess.check_output(['otool', '-tvj', 'hsw.o']).split('\n'):
  line = line.strip()
  if line == '' or line == 'hsw.o:' or line == '(__TEXT,__text) section':
    continue

  m = re.match('_(.*):', line)
  if m:
    name = m.group(1)
    print 'static const unsigned char kSplice_' + m.group(1) + '[] = {'
    continue

  # Skip the leading 16 byte address and a tab,
  # leaving the code, instruction mnemonic, and its arguments.
  line = line[17:]
  columns = line.split('\t')
  code = columns[0].strip()
  inst = columns[1]
  args = columns[2:]

  # We can't splice code that uses rip relative addressing.
  for arg in args:
    assert 'rip' not in arg

  # jmp done, the end of each stage (the address of done is not yet filled in)
  if code == 'e9 00 00 00 00':
    print '};'
    continue

  sys.stdout.write('    ')
  _bytes = code.split(' ')
  # This is the meat of things: copy the code to a C unsigned char array.
  for byte in _bytes:
    sys.stdout.write('0x' + byte + ',')
  # From here on we're just making the generated file readable and pretty.
  sys.stdout.write(' ' * (44 - 5*len(_bytes)))
  sys.stdout.write('// ' + inst)
  if args:
    sys.stdout.write(' ' * (13 - len(inst)))
    sys.stdout.write(' '.join(args))
  sys.stdout.write('\n')

print '''
#endif
'''

print '''#endif//SkSplicer_generated_DEFINED'''
