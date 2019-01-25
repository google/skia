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
        assert (len(mm) % s.size) == 0
        for offset in xrange(0, len(mm), s.size):
            yield s.unpack_from(mm, offset)[0]


def convert(fmt, name, src_path, dst_path):
    header, line_begin, line_end, footer = fmt
    assert os.path.exists(src_path)
    src = iterate_dwords(src_path)
    with open(dst_path, 'w') as o:
        o.write(header.format(name))
        while True:
            line = ','.join('%d' % v for _, v in zip(range(8), src))
            if not line:
                break
            o.write('%s%s%s\n' % (line_begin, line, line_end))
        o.write(footer.format(name))


gcc_asm = (
    '.globl {0}\n\t.balign 16\n#ifdef U_HIDE_DATA_SYMBOL\n\t.hidden {0}\n'
    '#endif\n{0}:\n\n',
    '.long ',
    '',
    '\n',
)

cpp = ( '#include <cstdint>\nextern "C" uint32_t {0}[] = {{\n', '', ',', '}};\n')

def main(argv):
    print '\n'.join('>>>  %r' % x for x in argv)
    name, current_os, target_cpu = argv[1], argv[2], argv[3]
    src, dst = argv[4], argv[5]
    wasm = target_cpu == 'wasm'
    fmt = gcc_asm if not wasm else cpp
    if not wasm and current_os in ['mac', 'ios', 'tvos']:
        name = '_' + name
    convert(fmt, name, src, dst)

if __name__ == '__main__':
    main(sys.argv)
