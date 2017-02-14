#!/usr/bin/env python2.7
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re
import subprocess
import sys

sys.stdout = open('src/splicer/SkSplicer_generated.h', 'w')

ndk = '/Users/mtklein/brew/opt/android-ndk/'
objdump = 'gobjdump'

#ndk = '/home/mtklein/ndk/'
#objdump = '/home/mtklein/binutils-2.27/binutils/objdump'

cflags = '-std=c++11 -Os -fomit-frame-pointer'.split()

sse2 = '-mno-red-zone -msse2 -mno-sse3 -mno-ssse3 -mno-sse4.1'.split()
subprocess.check_call(['clang++'] + cflags + sse2 +
                      ['-c', 'src/splicer/SkSplicer_stages.cpp'] +
                      ['-o', 'sse2.o'])

sse41 = '-mno-red-zone -msse4.1'.split()
subprocess.check_call(['clang++'] + cflags + sse41 +
                      ['-c', 'src/splicer/SkSplicer_stages.cpp'] +
                      ['-o', 'sse41.o'])


hsw = '-mno-red-zone -mavx2 -mfma -mf16c'.split()
subprocess.check_call(['clang++'] + cflags + hsw +
                      ['-c', 'src/splicer/SkSplicer_stages.cpp'] +
                      ['-o', 'hsw.o'])

aarch64 = [
    '--target=aarch64-linux-android',
    '--sysroot=' + ndk + 'platforms/android-21/arch-arm64',
]
subprocess.check_call(['clang++'] + cflags + aarch64 +
                      ['-c', 'src/splicer/SkSplicer_stages.cpp'] +
                      ['-o', 'aarch64.o'])

armv7 = [
    '--target=armv7a-linux-android',
    '--sysroot=' + ndk + 'platforms/android-18/arch-arm',
    '-mfpu=neon-vfpv4',
    '-mfloat-abi=hard',
]
subprocess.check_call(['clang++'] + cflags + armv7 +
                      ['-c', 'src/splicer/SkSplicer_stages.cpp'] +
                      ['-o', 'armv7.o'])

def parse_object_file(dot_o, array_type, jump, target=None):
  prefix = dot_o.replace('.o', '_')
  cmd = [ objdump, '-d', '--insn-width=8', dot_o]
  if target:
    cmd += ['--target', target]
  for line in subprocess.check_output(cmd).split('\n'):
    line = line.strip()
    if not line or line.startswith(dot_o) or line.startswith('Disassembly'):
      continue

    # E.g. 00000000000003a4 <_load_f16>:
    m = re.match('''[0-9a-f]+ <_?(.*)>:''', line)
    if m:
      print 'static const', array_type, prefix + m.group(1) + '[] = {'
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

    # At the end of every stage function there's a jump to next().
    # This marks the splice point.
    if code == jump:
      print '};'
      continue

    hexed = ''.join('0x'+x+',' for x in code.split(' '))
    print '    ' + hexed + ' '*(44-len(hexed)) + \
          '//  ' + inst  + (' '*(14-len(inst)) + args if args else '')

print '''/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSplicer_generated_DEFINED
#define SkSplicer_generated_DEFINED

// This file is generated semi-automatically with this command:
//   $ src/splicer/build_stages.py
'''
parse_object_file('aarch64.o', 'unsigned int', '14000000')
parse_object_file(  'armv7.o', 'unsigned int', 'eafffffe',
                  target='elf32-littlearm')
parse_object_file( 'sse2.o', 'unsigned char', 'e9 00 00 00 00')
#parse_object_file('sse41.o', 'unsigned char', 'e9 00 00 00 00')
parse_object_file(  'hsw.o', 'unsigned char', 'e9 00 00 00 00')
print '#endif//SkSplicer_generated_DEFINED'
