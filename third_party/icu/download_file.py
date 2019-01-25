#! /usr/bin/env python
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''
Download from URL to PATH only if the MD5 checksum does not match.
'''

import os
import sys
import urllib

import hasher

def download(url, md5, path):
    if md5 == hasher.hash_file('md5', path):
        return
    dirname = os.path.dirname(path)
    if dirname and not os.path.exists(dirname):
        os.makedirs(dirname)
    urllib.urlretrieve(url, path)
    assert md5 == hasher.hash_file('md5', path)

if __name__ == '__main__':
    print '\n'.join('>>>  %r' % a for a in sys.argv)
    download(sys.argv[1], sys.argv[2], sys.argv[3])
