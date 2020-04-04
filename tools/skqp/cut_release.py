#! /usr/bin/env python
# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

import json
import md5
import multiprocessing
import os
import shutil
import sys
import tempfile
import urllib
import urllib2

from subprocess import check_call, check_output

assert '/' in [os.sep, os.altsep] and os.pardir == '..'

ASSETS = 'platform_tools/android/apps/skqp/src/main/assets'
BUCKET = 'skia-skqp-assets'

def urlopen(url):
    cookie = os.environ.get('SKIA_GOLD_COOKIE', '')
    return urllib2.urlopen(urllib2.Request(url, headers={'Cookie': cookie}))

def make_skqp_model(arg):
    name, urls, exe = arg
    tmp = tempfile.mkdtemp()
    for url in urls:
        urllib.urlretrieve(url, tmp + '/' + url[url.rindex('/') + 1:])
    check_call([exe, tmp, ASSETS + '/gmkb/' + name])
    shutil.rmtree(tmp)
    sys.stdout.write(name + ' ')
    sys.stdout.flush()

def goldgetter(meta, exe):
    assert os.path.exists(exe)
    jobs = []
    for rec in meta:
        urls = [d['URL'] for d in rec['digests']
                if d['status'] == 'positive' and
                (set(d['paramset']['config']) & set(['vk', 'gles']))]
        if urls:
            jobs.append((rec['testName'], urls, exe))
    pool = multiprocessing.Pool(processes=20)
    pool.map(make_skqp_model, jobs)
    sys.stdout.write('\n')
    return set((n for n, _, _ in jobs))

def gold(first_commit, last_commit):
    c1, c2 = (check_output(['git', 'rev-parse', c]).strip()
            for c in (first_commit, last_commit))
    f = urlopen('https://public-gold.skia.org/json/export?' + urllib.urlencode([
        ('fbegin', c1),
        ('fend', c2),
        ('query', 'config=gles&config=vk&source_type=gm'),
        ('pos', 'true'),
        ('neg', 'false'),
        ('unt', 'false')
    ]))
    j = json.load(f)
    f.close()
    return j

def gset(path):
    s = set()
    if os.path.isfile(path):
        with open(path, 'r') as f:
            for line in f:
                s.add(line.strip())
    return s

def make_rendertest_list(models, good, bad):
    assert good.isdisjoint(bad)
    do_score = good & models
    no_score = bad | (good - models)
    to_delete = models & bad
    for d in to_delete:
        path = ASSETS + '/gmkb/' + d
        if os.path.isdir(path):
            shutil.rmtree(path)
    results = dict()
    for n in do_score:
        results[n] = 0
    for n in no_score:
        results[n] = -1
    return ''.join('%s,%d\n' % (n, results[n]) for n in sorted(results))

def get_digest(path):
    m = md5.new()
    with open(path, 'r') as f:
        m.update(f.read())
    return m.hexdigest()

def upload_cmd(path, digest):
    return ['gsutil', 'cp', path, 'gs://%s/%s' % (BUCKET, digest)]

def upload_model():
    bucket_url = 'gs://%s/' % BUCKET
    extant = set((u.replace(bucket_url, '', 1)
                  for u in check_output(['gsutil', 'ls', bucket_url]).splitlines() if u))
    cmds = []
    filelist = []
    for dirpath, _, filenames in os.walk(ASSETS + '/gmkb'):
        for filename in filenames:
            path = os.path.join(dirpath, filename)
            digest = get_digest(path)
            if digest not in extant:
                cmds.append(upload_cmd(path, digest))
            filelist.append('%s;%s\n' % (digest, os.path.relpath(path, ASSETS)))
    tmp = tempfile.mkdtemp()
    filelist_path = tmp + '/x'
    with open(filelist_path, 'w') as o:
        for l in filelist:
            o.write(l)
    filelist_digest = get_digest(filelist_path)
    if filelist_digest not in extant:
        cmds.append(upload_cmd(filelist_path, filelist_digest))

    pool = multiprocessing.Pool(processes=20)
    pool.map(check_call, cmds)
    shutil.rmtree(tmp)
    return filelist_digest

def remove(x):
    if os.path.isdir(x) and not os.path.islink(x):
        shutil.rmtree(x)
    if os.path.exists(x):
        os.remove(x)

def main(first_commit, last_commit):
    check_call(upload_cmd('/dev/null', get_digest('/dev/null')))

    os.chdir(os.path.dirname(__file__) + '/../..')
    remove(ASSETS + '/files.checksum')
    for d in [ASSETS + '/gmkb', ASSETS + '/skqp', ]:
        remove(d)
        os.mkdir(d)

    check_call([sys.executable, 'tools/git-sync-deps'],
               env=dict(os.environ, GIT_SYNC_DEPS_QUIET='T'))
    build = 'out/ndebug'
    check_call(['bin/gn', 'gen', build,
                '--args=cc="clang" cxx="clang++" is_debug=false'])
    check_call(['ninja', '-C', build,
                'jitter_gms', 'list_gpu_unit_tests', 'make_skqp_model'])

    models = goldgetter(gold(first_commit, last_commit), build + '/make_skqp_model')

    check_call([build + '/jitter_gms', 'tools/skqp/bad_gms.txt'])

    with open(ASSETS + '/skqp/rendertests.txt', 'w') as o:
        o.write(make_rendertest_list(models, gset('good.txt'), gset('bad.txt')))

    remove('good.txt')
    remove('bad.txt')

    with open(ASSETS + '/skqp/unittests.txt', 'w') as o:
        o.write(check_output([build + '/list_gpu_unit_tests']))

    with open(ASSETS + '/files.checksum', 'w') as o:
        o.write(upload_model() + '\n')

    sys.stdout.write(ASSETS + '/files.checksum\n')
    sys.stdout.write(ASSETS + '/skqp/rendertests.txt\n')
    sys.stdout.write(ASSETS + '/skqp/unittests.txt\n')

if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.stderr.write('Usage:\n  %s C1 C2\n\n' % sys.argv[0])
        sys.exit(1)
    main(sys.argv[1], sys.argv[2])
