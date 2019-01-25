#! /usr/bin/env python

# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
Generate a Windows object file containing the given binary data.
'''

import sys
import mmap
import struct
import time

def write_windows_obj(symbol_name, target_cpu, src_path, dst_path):
    machine_codes = {'x86'   : 0x014C,
                     'x64'   : 0x8664,
                     'arm'   : 0x01C0,
                     'arm64' : 0xAA64}
    if target_cpu == 'x86':
        symbol_name = '_' + symbol_name
    assert target_cpu in machine_codes
    src_file = open(src_path, 'rb')
    src = mmap.mmap(src_file.fileno(), 0, access=mmap.ACCESS_READ)
    image_file_header = struct.Struct('HHIIIHH')
    image_section_header = struct.Struct('8sIIIIIIHHI')
    linker_options = '-export:' + symbol_name + ',data '
    header_len = image_file_header.size + 2 * image_section_header.size
    opts_length = len(linker_options)
    file_header = image_file_header.pack(machine_codes[target_cpu], 2,
            int(time.time()), header_len + opts_length + len(src), 1, 0, 0)
    section1_header = image_section_header.pack(
            '.drectve', 0, 0, opts_length, header_len,
            0, 0, 0, 0, 0x00100A00)
    section2_header = image_section_header.pack(
            '.rdata', 0, 0, len(src), header_len + opts_length,
            0, 0, 0, 0, 0x40100000)
    symbols = struct.pack('IIIHHBB', 0, 4, 0, 2, 0, 2, 0)
    symbol_names = struct.pack('I', 5 + len(symbol_name)) + symbol_name + '\000'
    with open(dst_path, 'wb') as o:
        o.write(file_header)
        o.write(section1_header)
        o.write(section2_header)
        o.write(linker_options)
        o.write(src)
        o.write(symbols)
        o.write(symbol_names)
    src.close()
    src_file.close()


if __name__ == '__main__':
    print '\n'.join('>>>  %r' % a for a in sys.argv)
    write_windows_obj(sys.argv[1], sys.argv[3], sys.argv[4], sys.argv[5])
