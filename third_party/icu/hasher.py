#! /usr/bin/env python
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
Return the checksum of FILENAME, using the given ALGORITHM
'''

import hashlib
import mmap
import os
import sys

def hash_file(algorithm, filename):
    assert algorithm in hashlib.algorithms
    if not os.path.exists(filename):
        return ''
    h = hashlib.new(algorithm)
    with open(filename, 'rb') as f:
        m = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
        h.update(m)
        m.close()
    return h.hexdigest()

if __name__ == '__main__':
    sys.stdout.write(hash_file(sys.argv[1], sys.argv[2]) + '\n')
