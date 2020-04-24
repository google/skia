#! /usr/bin/env python

# Copyright 2018 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import hashlib
import multiprocessing
import os
import shutil
import sys
import tempfile
import urllib2

def checksum(path):
    if not os.path.exists(path):
        return None
    m = hashlib.md5()
    with open(path, 'rb') as f:
        while True:
            buf = f.read(4096)
            if not buf:
                return m.hexdigest()
            m.update(buf)

def download(md5, path):
    if not md5 == checksum(path):
        dirname = os.path.dirname(path)
        if dirname and not os.path.exists(dirname):
            try:
                os.makedirs(dirname)
            except OSError:
                pass  # ignore race condition
        url = 'https://storage.googleapis.com/skia-skqp-assets/' + md5
        with open(path, 'wb') as o:
            shutil.copyfileobj(urllib2.urlopen(url), o)

def tmp(prefix):
    fd, path = tempfile.mkstemp(prefix=prefix)
    os.close(fd)
    return path

def main():
    target_dir = os.path.join('platform_tools', 'android', 'apps', 'skqp', 'src', 'main', 'assets')
    os.chdir(os.path.join(os.path.dirname(__file__), os.pardir, os.pardir, target_dir))
    checksum_path = 'files.checksum'
    if not os.path.isfile(checksum_path):
        sys.stderr.write('Error: "%s" is missing.\n' % os.path.join(target_dir, checksum_path))
        sys.exit(1)
    file_list_file = tmp('files_')
    with open(checksum_path, 'r') as f:
        md5 = f.read().strip()
        assert(len(md5) == 32)
        download(md5, file_list_file)
    with open(file_list_file, 'r') as f:
        records = []
        for line in f:
            md5, path = line.strip().split(';', 1)
            records.append((md5, path))
    sys.stderr.write('Downloading %d files.\n' % len(records))
    pool = multiprocessing.Pool(processes=multiprocessing.cpu_count() * 2)
    for record in records:
        pool.apply_async(download, record, callback=lambda x: sys.stderr.write('.'))
    pool.close()
    pool.join()
    sys.stderr.write('\n')

if __name__ == '__main__':
    main()
