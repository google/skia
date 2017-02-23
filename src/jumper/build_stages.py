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

cflags = '-std=c++11 -Os -fomit-frame-pointer -DJUMPER'.split()

sse2 = '-mno-red-zone -msse2 -mno-sse3 -mno-ssse3 -mno-sse4.1'.split()
subprocess.check_call(['clang++'] + cflags + sse2 +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'sse2.o'])
subprocess.check_call(['clang++'] + cflags + sse2 + ['-DWIN'] +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'win_sse2.o'])

sse41 = '-mno-red-zone -msse4.1'.split()
subprocess.check_call(['clang++'] + cflags + sse41 +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'sse41.o'])
subprocess.check_call(['clang++'] + cflags + sse41 + ['-DWIN'] +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'win_sse41.o'])

avx = '-mno-red-zone -mavx'.split()
subprocess.check_call(['clang++'] + cflags + avx +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'avx.o'])
subprocess.check_call(['clang++'] + cflags + avx + ['-DWIN'] +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'win_avx.o'])

hsw = '-mno-red-zone -mavx2 -mfma -mf16c'.split()
subprocess.check_call(['clang++'] + cflags + hsw +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'hsw.o'])
subprocess.check_call(['clang++'] + cflags + hsw + ['-DWIN'] +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'win_hsw.o'])

aarch64 = [
    '--target=aarch64-linux-android',
    '--sysroot=' + ndk + 'platforms/android-21/arch-arm64',
]
subprocess.check_call(['clang++'] + cflags + aarch64 +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'aarch64.o'])

vfp4 = [
    '--target=armv7a-linux-android',
    '--sysroot=' + ndk + 'platforms/android-18/arch-arm',
    '-mfpu=neon-vfpv4',
    '-mfloat-abi=hard',
]
subprocess.check_call(['clang++'] + cflags + vfp4 +
                      ['-c', 'src/jumper/SkJumper_stages.cpp'] +
                      ['-o', 'vfp4.o'])

def parse_object_file(dot_o, directive, target=None):
  globl, label, comment = '.globl', ':', '// '
  if 'win' in dot_o:
    globl, label, comment = 'PUBLIC', ' LABEL PROC', '; '

  dehex = lambda h: '0x'+h
  if directive != '.long':
    dehex = lambda h: str(int(h, 16))

  cmd = [ objdump, '-d', '--insn-width=9', dot_o]
  if target:
    cmd += ['--target', target]

  for line in subprocess.check_output(cmd).split('\n'):
    line = line.strip()

    if not line or line.startswith(dot_o) or line.startswith('Disassembly'):
      continue

    # E.g. 00000000000003a4 <_load_f16>:
    m = re.match('''[0-9a-f]+ <_?(.*)>:''', line)
    if m:
      print
      print globl + ' _' + m.group(1)
      print '_' + m.group(1) + label
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

    hexed = ','.join(dehex(x) for x in code.split(' '))

    print '  ' + directive + '  ' + hexed + ' '*(36-len(hexed)) + \
          comment + inst  + (' '*(14-len(inst)) + args if args else '')

sys.stdout = open('src/jumper/SkJumper_generated.S', 'w')

print '''# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is generated semi-automatically with this command:
#   $ src/jumper/build_stages.py
'''
print '.text'

print '#if defined(__aarch64__)'
print '.balign 4'
parse_object_file('aarch64.o', '.long')

print '#elif defined(__arm__)'
print '.balign 4'
parse_object_file('vfp4.o', '.long', target='elf32-littlearm')

print '#elif defined(__x86_64__)'
parse_object_file('hsw.o',   '.byte')
parse_object_file('avx.o',   '.byte')
parse_object_file('sse41.o', '.byte')
parse_object_file('sse2.o',  '.byte')
print '#endif'

sys.stdout = open('src/jumper/SkJumper_generated_win.S', 'w')

print '''; Copyright 2017 Google Inc.
;
; Use of this source code is governed by a BSD-style license that can be
; found in the LICENSE file.

; This file is generated semi-automatically with this command:
;   $ src/jumper/build_stages.py
'''
print '_text SEGMENT'
parse_object_file('win_hsw.o',   'DB')
parse_object_file('win_avx.o',   'DB')
parse_object_file('win_sse41.o', 'DB')
parse_object_file('win_sse2.o',  'DB')
print 'END'
