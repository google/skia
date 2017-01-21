#!/usr/bin/env python2.7
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re
import subprocess
import sys

ndk = '/Users/mtklein/brew/opt/android-ndk/'
objdump = 'gobjdump'

#ndk = '/home/mtklein/ndk/'
#objdump = '/home/mtklein/binutils-2.27/binutils/objdump'

cflags = '-std=c++11 -Os -fomit-frame-pointer'.split()

hsw = '-mavx2 -mfma -mf16c'.split()
subprocess.check_call(['clang++'] + cflags + hsw +
                      ['-c', 'src/splicer/SkSplicer_stages.cpp'] +
                      ['-o', 'hsw.o'])
subprocess.check_call(['clang++'] + cflags + hsw +
                      ['-c', 'src/splicer/SkSplicer_stages_lowp.cpp'] +
                      ['-o', 'hsw_lowp.o'])

aarch64 = [
    '--target=aarch64-linux-android',
    '--sysroot=' + ndk + 'platforms/android-21/arch-arm64',
]
subprocess.check_call(['clang++'] + cflags + aarch64 +
                      ['-c', 'src/splicer/SkSplicer_stages.cpp'] +
                      ['-o', 'aarch64.o'])
subprocess.check_call(['clang++'] + cflags + aarch64 +
                      ['-c', 'src/splicer/SkSplicer_stages_lowp.cpp'] +
                      ['-o', 'aarch64_lowp.o'])

armv7 = [
    '--target=armv7a-linux-android',
    '--sysroot=' + ndk + 'platforms/android-18/arch-arm',
    '-mfpu=neon-vfpv4',
    '-mfloat-abi=hard',
]
subprocess.check_call(['clang++'] + cflags + armv7 +
                      ['-c', 'src/splicer/SkSplicer_stages.cpp'] +
                      ['-o', 'armv7.o'])
subprocess.check_call(['clang++'] + cflags + armv7 +
                      ['-c', 'src/splicer/SkSplicer_stages_lowp.cpp'] +
                      ['-o', 'armv7_lowp.o'])

def parse_object_file(dst, dot_o, array_type, done, target=None):
  cmd = [ objdump, '-d', dot_o]
  if target:
    cmd += ['--target', target]
  for line in subprocess.check_output(cmd).split('\n'):
    line = line.strip()
    if not line or line.startswith(dot_o) or line.startswith('Disassembly'):
      continue

    # E.g. 00000000000003a4 <_load_f16>:
    m = re.match('''[0-9a-f]+ <_?(.*)>:''', line)
    if m:
      print >>dst,'static const', array_type, 'kSplice_' + m.group(1) + '[] = {'
      continue

    columns = line.split('\t')
    code = columns[1]
    if len(columns) >= 4:
      inst = columns[2]
      args = columns[3]
    else:
      inst, args = columns[2].split(' ', 1)
    code, inst, args = code.strip(), inst.strip(), args.strip()

    # We can't splice code that uses ip-relative addressing.
    for arg in args:
      assert 'rip' not in arg  # TODO: detect on aarch64 too

    if code == done:
      print >>dst,'};'
      continue

    hexed = ''.join('0x'+x+',' for x in code.split(' '))
    print >>dst,'    ' + hexed + ' '*(44-len(hexed)) + \
                '//  ' + inst  + ' '*(14-len(inst))  + args

for suffix in ['', '_lowp']:
  with open('src/splicer/SkSplicer_generated%s.h' % suffix, 'w') as f:
    print >>f,'''/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSplicer_generated%s_DEFINED
#define SkSplicer_generated%s_DEFINED

// This file is generated semi-automatically with this command:
//   $ src/splicer/build_stages.py

#if defined(__aarch64__)
''' % (suffix, suffix)
    parse_object_file(f, 'aarch64%s.o' % suffix, 'unsigned int', '14000000')
    print >>f,'\n#elif defined(__ARM_NEON__)\n'
    parse_object_file(f, 'armv7%s.o' % suffix, 'unsigned int', 'eafffffe',
                    target='elf32-littlearm')
    print >>f,'\n#else\n'
    parse_object_file(f, 'hsw%s.o' % suffix, 'unsigned char', 'e9 00 00 00 00')
    print >>f,'\n#endif\n'
    print >>f,'#endif//SkSplicer_generated%s_DEFINED' % suffix
