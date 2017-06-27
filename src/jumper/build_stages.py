#!/usr/bin/env python2.7
#
# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re
import subprocess
import sys

clang         = 'clang-4.0'
objdump       = 'gobjdump'
ccache        = 'ccache'
stages        = 'src/jumper/SkJumper_stages.cpp'
stages_lowp   = 'src/jumper/SkJumper_stages_lowp.cpp'
generated     = 'src/jumper/SkJumper_generated.S'
generated_win = 'src/jumper/SkJumper_generated_win.S'

clang         = sys.argv[1] if len(sys.argv) > 1 else clang
objdump       = sys.argv[2] if len(sys.argv) > 2 else objdump
ccache        = sys.argv[3] if len(sys.argv) > 3 else ccache
stages        = sys.argv[4] if len(sys.argv) > 4 else stages
stages_lowp   = sys.argv[5] if len(sys.argv) > 5 else stages_lowp
generated     = sys.argv[6] if len(sys.argv) > 6 else generated
generated_win = sys.argv[7] if len(sys.argv) > 7 else generated_win

clang = [ccache, clang, '-x', 'c++']


cflags = ['-std=c++11', '-Os', '-DJUMPER',
          '-momit-leaf-frame-pointer', '-ffp-contract=fast',
          '-fno-exceptions', '-fno-rtti', '-fno-unwind-tables']

x86 = [ '-m32' ]
win = ['-DWIN', '-mno-red-zone']
sse2 = ['-msse2', '-mno-sse3', '-mno-ssse3', '-mno-sse4.1']
subprocess.check_call(clang + cflags + sse2 +
                      ['-c', stages] +
                      ['-o', 'sse2.o'])
subprocess.check_call(clang + cflags + sse2 + win +
                      ['-c', stages] +
                      ['-o', 'win_sse2.o'])
subprocess.check_call(clang + cflags + sse2 + x86 +
                      ['-c', stages] +
                      ['-o', 'x86_sse2.o'])
subprocess.check_call(clang + cflags + sse2 + win + x86 +
                      ['-c', stages] +
                      ['-o', 'win_x86_sse2.o'])

ssse3 = ['-mssse3', '-mno-sse4.1']
subprocess.check_call(clang + cflags + ssse3 +
                      ['-c', stages_lowp] +
                      ['-o', 'lowp_ssse3.o'])
subprocess.check_call(clang + cflags + ssse3 + win +
                      ['-c', stages_lowp] +
                      ['-o', 'win_lowp_ssse3.o'])

sse41 = ['-msse4.1']
subprocess.check_call(clang + cflags + sse41 +
                      ['-c', stages] +
                      ['-o', 'sse41.o'])
subprocess.check_call(clang + cflags + sse41 + win +
                      ['-c', stages] +
                      ['-o', 'win_sse41.o'])

avx = ['-mavx']
subprocess.check_call(clang + cflags + avx +
                      ['-c', stages] +
                      ['-o', 'avx.o'])
subprocess.check_call(clang + cflags + avx + win +
                      ['-c', stages] +
                      ['-o', 'win_avx.o'])

hsw = ['-mavx2', '-mfma', '-mf16c']
subprocess.check_call(clang + cflags + hsw +
                      ['-c', stages] +
                      ['-o', 'hsw.o'])
subprocess.check_call(clang + cflags + hsw + win +
                      ['-c', stages] +
                      ['-o', 'win_hsw.o'])
subprocess.check_call(clang + cflags + hsw +
                      ['-c', stages_lowp] +
                      ['-o', 'lowp_hsw.o'])
subprocess.check_call(clang + cflags + hsw + win +
                      ['-c', stages_lowp] +
                      ['-o', 'win_lowp_hsw.o'])

aarch64 = [ '--target=aarch64' ]
subprocess.check_call(clang + cflags + aarch64 +
                      ['-c', stages] +
                      ['-o', 'aarch64.o'])

vfp4 = [
    '--target=armv7a-linux-gnueabihf',
    '-mfpu=neon-vfpv4',
]
subprocess.check_call(clang + cflags + vfp4 +
                      ['-c', stages] +
                      ['-o', 'vfp4.o'])

def parse_object_file(dot_o, directive, target=None):
  globl, hidden, label, comment, align = \
      '.globl', 'HIDDEN', ':', '// ', 'BALIGN'
  if 'win' in dot_o:
    globl, hidden, label, comment, align = \
        'PUBLIC', '', ' LABEL PROC', '; ', 'ALIGN '

  cmd = [objdump]
  if target:
    cmd += ['--target', target]

  # Look for sections we know we can't handle.
  section_headers = subprocess.check_output(cmd + ['-h', dot_o])
  for snippet in ['.rodata']:
    if snippet in section_headers:
      print >>sys.stderr, 'Found %s in section.' % snippet
      assert snippet not in section_headers

  if directive == '.long':
    disassemble = ['-d', dot_o]
    dehex = lambda h: '0x'+h
  else:
    # x86-64... as long as we're using %rip-relative addressing,
    # literal sections should be fine to just dump in with .text.
    disassemble = ['-d',               # DO NOT USE -D.
                   '-z',               # Print zero bytes instead of ...
                   '--insn-width=11',
                   '-j', '.text',
                   '-j', '.literal4',
                   '-j', '.literal16',
                   '-j', '.const',
                   dot_o]
    dehex = lambda h: str(int(h,16))

  # Ok.  Let's disassemble.
  for line in subprocess.check_output(cmd + disassemble).split('\n'):
    line = line.strip()

    if not line or line.startswith(dot_o) or line.startswith('Disassembly'):
      continue

    # E.g. 00000000000003a4 <_load_f16>:
    m = re.match('''[0-9a-f]+ <_?(.*)>:''', line)
    if m:
      print
      sym = m.group(1)
      if sym.startswith('.literal'):  # .literal4, .literal16, etc
        print sym.replace('.literal', align)
      elif sym.startswith('.const'):  # 32-byte constants
        print align + '32'
      elif not sym.startswith('sk_'):
        print >>sys.stderr, "build_stages.py can't handle '%s' (yet?)." % sym
        assert sym.startswith('sk_')
      else:  # a stage function
        if hidden:
          print hidden + ' _' + sym
        print globl + ' _' + sym
        if 'win' not in dot_o:
          print 'FUNCTION(_' + sym + ')'
        print '_' + sym + label
      continue

    columns = line.split('\t')
   #print >>sys.stderr, columns
    code = columns[1]
    if len(columns) >= 4:
      inst = columns[2]
      args = columns[3]
    else:
      inst, args = columns[2], ''
      if ' ' in columns[2]:
        inst, args = columns[2].split(' ', 1)
    code, inst, args = code.strip(), inst.strip(), args.strip()

    hexed = ','.join(dehex(x) for x in code.split(' '))
    print '  ' + directive + '  ' + hexed + ' '*(36-len(hexed)) + \
          comment + inst + (' '*(14-len(inst)) + args if args else '')

sys.stdout = open(generated, 'w')

print '''# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is generated semi-automatically with this command:
#   $ src/jumper/build_stages.py
'''
print '#if defined(__MACH__)'
print '    #define HIDDEN .private_extern'
print '    #define FUNCTION(name)'
print '    #define BALIGN4  .align 2'
print '    #define BALIGN16 .align 4'
print '    #define BALIGN32 .align 5'
print '#else'
print '    .section .note.GNU-stack,"",%progbits'
print '    #define HIDDEN .hidden'
print '    #define FUNCTION(name) .type name,%function'
print '    #define BALIGN4  .balign 4'
print '    #define BALIGN16 .balign 16'
print '    #define BALIGN32 .balign 32'
print '#endif'

print '.text'
print '#if defined(__aarch64__)'
print 'BALIGN4'
parse_object_file('aarch64.o', '.long')

print '#elif defined(__arm__)'
print 'BALIGN4'
parse_object_file('vfp4.o', '.long', target='elf32-littlearm')

print '#elif defined(__x86_64__)'
print 'BALIGN32'
parse_object_file('hsw.o',   '.byte')
print 'BALIGN32'
parse_object_file('avx.o',   '.byte')
print 'BALIGN32'
parse_object_file('sse41.o', '.byte')
print 'BALIGN32'
parse_object_file('sse2.o',  '.byte')
print 'BALIGN32'
parse_object_file('lowp_hsw.o',  '.byte')
print 'BALIGN32'
parse_object_file('lowp_ssse3.o',  '.byte')

print '#elif defined(__i386__)'
print 'BALIGN32'
parse_object_file('x86_sse2.o', '.byte')

print '#endif'

sys.stdout = open(generated_win, 'w')
print '''; Copyright 2017 Google Inc.
;
; Use of this source code is governed by a BSD-style license that can be
; found in the LICENSE file.

; This file is generated semi-automatically with this command:
;   $ src/jumper/build_stages.py
'''
print 'IFDEF RAX'
print "_text32 SEGMENT ALIGN(32) 'CODE'"
print 'ALIGN 32'
parse_object_file('win_hsw.o',   'DB')
print 'ALIGN 32'
parse_object_file('win_avx.o',   'DB')
print 'ALIGN 32'
parse_object_file('win_sse41.o', 'DB')
print 'ALIGN 32'
parse_object_file('win_sse2.o',  'DB')
print 'ALIGN 32'
parse_object_file('win_lowp_hsw.o',  'DB')
print 'ALIGN 32'
parse_object_file('win_lowp_ssse3.o',  'DB')

print 'ELSE'
print '.MODEL FLAT,C'
print "_text32 SEGMENT ALIGN(32) 'CODE'"
print 'ALIGN 32'
parse_object_file('win_x86_sse2.o', 'DB')

print 'ENDIF'
print 'END'
