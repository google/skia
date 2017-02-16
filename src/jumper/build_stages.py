#!/usr/bin/env python2.7
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re
import subprocess
import sys

sys.stdout = open('src/jumper/SkJumper_generated.h', 'w')

ndk = '/Users/mtklein/brew/opt/android-ndk/'
objdump = 'gobjdump'

#ndk = '/home/mtklein/ndk/'
#objdump = '/home/mtklein/binutils-2.27/binutils/objdump'

cflags = '-std=c++11 -Os -fomit-frame-pointer -DJUMPER'.split()

sse2 = '-mno-red-zone -msse2 -mno-sse3 -mno-ssse3 -mno-sse4.1'.split()
subprocess.check_call(['clang++'] + cflags + sse2 +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'sse2.o'])

sse41 = '-mno-red-zone -msse4.1'.split()
subprocess.check_call(['clang++'] + cflags + sse41 +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'sse41.o'])

hsw = '-mno-red-zone -mavx2 -mfma -mf16c'.split()
subprocess.check_call(['clang++'] + cflags + hsw +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'hsw.o'])

aarch64 = [
    '--target=aarch64-linux-android',
    '--sysroot=' + ndk + 'platforms/android-21/arch-arm64',
]
subprocess.check_call(['clang++'] + cflags + aarch64 +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'aarch64.o'])

armv7 = [
    '--target=armv7a-linux-android',
    '--sysroot=' + ndk + 'platforms/android-18/arch-arm',
    '-mfpu=neon-vfpv4',
    '-mfloat-abi=hard',
]
subprocess.check_call(['clang++'] + cflags + armv7 +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'armv7.o'])

def parse_object_file(dot_o, array_type, target=None):
  prefix = dot_o.replace('.o', '_')
  cmd = [ objdump, '-d', '--insn-width=9', dot_o]
  if target:
    cmd += ['--target', target]

  active = False
  for line in subprocess.check_output(cmd).split('\n'):
    line = line.strip()

    if line.startswith(dot_o) or line.startswith('Disassembly'):
      continue

    if not line:
      if active:
        print '};'
        active = False
      continue

    # E.g. 00000000000003a4 <_load_f16>:
    m = re.match('''[0-9a-f]+ <_?(.*)>:''', line)
    if m:
      print 'static const', array_type, prefix + m.group(1) + '[] = {'
      active = True
      continue

    columns = line.split('\t')
    code = columns[1]
    if len(columns) >= 4:
      inst = columns[2]
      args = columns[3]
    else:
      inst, args = columns[2], ''
      if ' ' in columns[2]:
        inst, args = columns[2].split(' ', 1)
    code, inst, args = code.strip(), inst.strip(), args.strip()

    # We can't work with code that uses ip-relative addressing.
    for arg in args:
      assert 'rip' not in arg  # TODO: detect on aarch64 too

    hexed = ''.join('0x'+x+',' for x in code.split(' '))
    print '    ' + hexed + ' '*(48-len(hexed)) + \
          '//  ' + inst  + (' '*(14-len(inst)) + args if args else '')

print '''/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJumper_generated_DEFINED
#define SkJumper_generated_DEFINED

// This file is generated semi-automatically with this command:
//   $ src/jumper/build_stages.py
'''
parse_object_file('aarch64.o', 'unsigned int')
parse_object_file('armv7.o',   'unsigned int', target='elf32-littlearm')
parse_object_file('hsw.o',     'unsigned char')
parse_object_file('sse41.o',   'unsigned char')
parse_object_file('sse2.o',    'unsigned char')
print '#endif//SkJumper_generated_DEFINED'
