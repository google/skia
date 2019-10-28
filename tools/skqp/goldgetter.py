#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import multiprocessing
import os
import shutil
import subprocess
import sys
import tempfile
import urllib

def make_skqp_model(arg):
    name, urls, dst_dir, exe = arg
    tmp = tempfile.mkdtemp()
    for url in urls:
        urllib.urlretrieve(url, tmp + '/' + url[url.rindex('/') + 1:])
    subprocess.check_call([exe, tmp, dst_dir + '/' + name])
    shutil.rmtree(tmp)
    sys.stdout.write(name + ' ')
    sys.stdout.flush()

def main(meta, dst, exe):
    assert os.path.exists(exe)
    jobs = []
    with open(meta, 'r') as f:
        for rec in json.load(f):
            urls = [d['URL'] for d in rec['digests']
                    if d['status'] == 'positive' and
                    (set(d['paramset']['config']) & set(['vk', 'gles']))]
            if urls:
                jobs.append((rec['testName'], urls, dst, exe))
    if not os.path.exists(dst):
        os.mkdir(dst)
    pool = multiprocessing.Pool(processes=20)
    pool.map(make_skqp_model, jobs)
    sys.stdout.write('\n')
    with open(dst + '/models.txt', 'w') as o:
        for n, _, _, _ in jobs:
            o.write(n + '\n')

if __name__ == '__main__':
    if len(sys.argv) != 4:
        sys.stderr.write('Usage:\n  %s META.JSON DST_DIR MAKE_SKQP_MODEL_EXE\n\n' % sys.argv[0])
        sys.exit(1)
    main(sys.argv[1], sys.argv[2], sys.argv[3])
