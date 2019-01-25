#!/usr/bin/env python

# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import struct
import sys

def convert(src_path, dst_path):
    assert os.path.exists(src_path)
    s = struct.Struct('<L')
    with open(dst_path, 'wb') as o:
        with open(src_path, 'r') as f:
            beginning = '.long '
            for line in f:
                if line.startswith(beginning):
                    line = line.strip()[len(beginning):]
                    for number in line.split(','):
                        o.write(s.pack(int(number, 0)))


if __name__ == '__main__':
    convert(sys.argv[1], sys.argv[2])
