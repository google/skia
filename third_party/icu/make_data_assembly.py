#!/usr/bin/env python

# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
Generate a source file containing the given binary data.

File type is either assembly or C++, depending on platform.
'''

import os
import struct
import sys
import mmap

def iterate_as_uint32(path):
    with open(path, 'rb') as f:
        s = struct.Struct('@I')
        assert s.size == 4
        mm = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
        assert (len(mm) % s.size) == 0
        for offset in xrange(0, len(mm), s.size):
            yield s.unpack_from(mm, offset)[0]
        mm.close()


def convert(fmt, name, src_path, dst_path):
    header, line_begin, line_end, footer = fmt
    assert os.path.exists(src_path)
    src = iterate_as_uint32(src_path)
    with open(dst_path, 'w') as o:
        o.write(header.format(name))
        while True:
            line = ','.join('%d' % v for _, v in zip(range(8), src))
            if not line:
                break
            o.write('%s%s%s\n' % (line_begin, line, line_end))
        o.write(footer.format(name))


gcc_asm = ('.globl {0}\n.balign 16\n{0}:\n', '.long ', '', '')

cpp = ('#include <cstdint>\nextern "C" uint32_t {0}[] __attribute__((aligned(16))) = {{\n',
       '', ',', '}};\n')

def main(name, current_os, target_cpu, src, dst):
    wasm = target_cpu == 'wasm'
    fmt = gcc_asm if not wasm else cpp
    if not wasm and current_os in ['mac', 'ios', 'tvos']:
        name = '_' + name
    convert(fmt, name, src, dst)

if __name__ == '__main__':
    print '\n'.join('>>>  %r' % x for x in sys.argv)
    main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
