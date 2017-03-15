#!/usr/bin/env python2.7
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re
import subprocess
import sys

#clang = ['clang++']
clang = ['ccache', 'clang-4.0', '-x', 'c++']

ndk = '/Users/mtklein/brew/opt/android-ndk/'
objdump = 'gobjdump'

#ndk = '/home/mtklein/ndk/'
#objdump = '/home/mtklein/binutils-2.27/binutils/objdump'

cflags = ['-std=c++11', '-Os', '-DJUMPER',
          '-fomit-frame-pointer', '-ffp-contract=fast' ]

sse2 = '-mno-red-zone -msse2 -mno-sse3 -mno-ssse3 -mno-sse4.1'.split()
subprocess.check_call(clang + cflags + sse2 +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'sse2.o'])
subprocess.check_call(clang + cflags + sse2 + ['-DWIN'] +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'win_sse2.o'])

sse41 = '-mno-red-zone -msse4.1'.split()
subprocess.check_call(clang + cflags + sse41 +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'sse41.o'])
subprocess.check_call(clang + cflags + sse41 + ['-DWIN'] +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'win_sse41.o'])

avx = '-mno-red-zone -mavx'.split()
subprocess.check_call(clang + cflags + avx +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'avx.o'])
subprocess.check_call(clang + cflags + avx + ['-DWIN'] +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'win_avx.o'])

hsw = '-mno-red-zone -mavx2 -mfma -mf16c'.split()
subprocess.check_call(clang + cflags + hsw +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'hsw.o'])
subprocess.check_call(clang + cflags + hsw + ['-DWIN'] +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'win_hsw.o'])

aarch64 = [
    '--target=aarch64-linux-android',
    '--sysroot=' + ndk + 'platforms/android-21/arch-arm64',
]
subprocess.check_call(clang + cflags + aarch64 +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'aarch64.o'])

vfp4 = [
    '--target=armv7a-linux-android',
    '--sysroot=' + ndk + 'platforms/android-18/arch-arm',
    '-mfpu=neon-vfpv4',
    '-mfloat-abi=hard',
]
subprocess.check_call(clang + cflags + vfp4 +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'vfp4.o'])

def parse_object_file(dot_o, array_type, target=None):
  cmd = [objdump]
  if target:
    cmd += ['--target', target]

  # Look for sections we know we can't handle.
  section_headers = subprocess.check_output(cmd + ['-h', dot_o])
  for snippet in ['.literal', '.const', '.rodata']:
    if snippet in section_headers:
      print >>sys.stderr, 'Found %s in section.' % snippet
      assert snippet not in section_headers

  # Ok.  Let's disassemble.
  active = False
  disassemble = ['-d', '--insn-width=10', dot_o]
  for line in subprocess.check_output(cmd + disassemble).split('\n'):
    line = line.strip()

    if not line or line.startswith(dot_o) or line.startswith('Disassembly'):
      continue

    # E.g. 00000000000003a4 <_load_f16>:
    m = re.match('''[0-9a-f]+ <_?(.*)>:''', line)
    if m:
      if active:
        print '};'
      print
      print 'CODE const', array_type, m.group(1) + '[] = {'
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

    dehex = lambda x: '0x'+x
    if array_type == 'uint8_t':
      dehex = lambda x: str(int(x, 16))

    hexed = ''.join(dehex(x) + ',' for x in code.split(' '))
    print '  ' + hexed + ' '*(40-len(hexed)) + \
          '//' + inst  + (' '*(14-len(inst)) + args if args else '')
  print '};'

sys.stdout = open('src/jumper/SkJumper_generated.cpp', 'w')

print '''/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This file is generated semi-automatically with this command:
//   $ src/jumper/build_stages.py

#include <stdint.h>

#if defined(_MSC_VER)
    #pragma section("code", read,execute)
    #define CODE extern "C" __declspec(allocate("code"))
#elif defined(__MACH__)
    #define CODE extern "C" __attribute__((section("__TEXT,__text")))
#else
    #define CODE extern "C" __attribute__((section(".text")))
#endif
'''
print '#if defined(__aarch64__)'
parse_object_file('aarch64.o', 'uint32_t')

print '#elif defined(__arm__)'
parse_object_file('vfp4.o', 'uint32_t', target='elf32-littlearm')

print '#elif defined(__x86_64__)'
parse_object_file('hsw.o',   'uint8_t')
parse_object_file('avx.o',   'uint8_t')
parse_object_file('sse41.o', 'uint8_t')
parse_object_file('sse2.o',  'uint8_t')

print '#elif defined(_M_X64)'
parse_object_file('win_hsw.o',   'uint8_t')
parse_object_file('win_avx.o',   'uint8_t')
parse_object_file('win_sse41.o', 'uint8_t')
parse_object_file('win_sse2.o',  'uint8_t')

print '#endif'
