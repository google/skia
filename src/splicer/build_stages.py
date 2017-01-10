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

def parse_object_file(dot_o, array_type, done):
  for line in subprocess.check_output(['gobjdump', '-d', dot_o]).split('\n'):
    line = line.strip()
    if not line or line.startswith(dot_o) or line.startswith('Disassembly'):
      continue

    # E.g. 00000000000003a4 <_load_f16>:
    m = re.match('''................ <_?(.*)>:''', line)
    if m:
      print 'static const', array_type, 'kSplice_' + m.group(1) + '[] = {'
      continue

    columns = line.split('\t')
    code = columns[1]
    if len(columns) == 4:
      inst = columns[2]
      args = columns[3]
    else:
      inst, args = columns[2].split(' ', 1)
    code, inst, args = code.strip(), inst.strip(), args.strip()

    # We can't splice code that uses ip-relative addressing.
    for arg in args:
      assert 'rip' not in arg  # TODO: detect on aarch64 too

    if code == done:
      print '};'
      continue

    hexed = ''.join('0x'+x+',' for x in code.split(' '))
    print '    ' + hexed + ' '*(44-len(hexed)) + \
          '//  ' + inst  + ' '*(14-len(inst))  + args

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
parse_object_file('aarch64.o', 'unsigned int', '14000000')
print '\n#else\n'
parse_object_file('hsw.o', 'unsigned char', 'e9 00 00 00 00')
print '\n#endif\n'
print '#endif//SkSplicer_generated_DEFINED'
