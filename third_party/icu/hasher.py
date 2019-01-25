#! /usr/bin/env python
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import hashlib
import mmap
import os
import sys

def hash_file(algo, path):
    assert algo in hashlib.algorithms
    if not os.path.exists(path):
        return ''
    h = hashlib.new(algo)
    with open(path, 'rb') as f:
        m = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
        h.update(m)
        m.close()
    return h.hexdigest()

if __name__ == '__main__':
    sys.stdout.write(hash_file(sys.argv[1], sys.argv[2]) + '\n')
