#!/usr/bin/env python

# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import struct
import sys
import mmap

def iterate_dwords(path):
    with open(path, 'rb') as f:
        s = struct.Struct('@I')
        assert s.size == 4
        mm = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
        assert 0 == (len(mm) % s.size)
        for offset in xrange(0, len(mm), s.size):
            yield s.unpack_from(mm, offset)[0]


def convert(fmt, name, src_path, dst_path):
    header, line_begin, footer = fmt
    assert os.path.exists(src_path)
    src = iterate_dwords(src_path)
    with open(dst_path, 'w') as o:
        o.write(header.format(name))
        while True:
            line = ','.join('%d' % v for _, v in zip(range(8), src))
            if not line:
                break
            o.write('%s%s\n' % (line_begin, line))
        o.write('\n' + footer.format(name))


gcc_asm = (
    '.globl {0}\n\t.balign 16\n#ifdef U_HIDE_DATA_SYMBOL\n\t.hidden {0}\n'
    '#endif\n{0}:\n\n',
    '.long ',
    '',
)

ms_asm = (
    "\tTITLE {0}\n.386\n.model flat\n\tPUBLIC _{0}\n"
    "ICUDATA_{0}\tSEGMENT READONLY PARA PUBLIC FLAT 'DATA'\n\tALIGN 16\n"
    "_{0}\tLABEL DWORD\n",
    '\tDWORD ',
    '\nICUDATA_{0}\tENDS\n\tEND\n',
)

ms_asm_64 = (
    "INCLUDELIB OLDNAMES\nPUBLIC\t{0}\n"
    "_DATA\tSEGMENT\n{0}",
    "\tDD\t",
    "_DATA\tENDS\nEND\n"
)

if __name__ == '__main__':
    print '\n'.join('>>>  %r' % x for x in sys.argv)
    name, current_os, target_cpu, src, dst = sys.argv[1:6]
    if current_os == 'win' and target_cpu in [ 'x64', 'arm64' ]:
        fmt = ms_asm_64
    elif current_os == 'win':
        fmt = ms_asm
    else:
        fmt = gcc_asm
    if current_os == 'mac':
        name = '_' + name
    convert(fmt, name, src, dst)
