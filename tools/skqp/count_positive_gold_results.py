#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import collections
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import urllib

def get_jobs():
    skia_directory = os.path.dirname(__file__) + '/../..'
    path = 'infra/bots/jobs.json'
    reg = re.compile('Test-(?P<os>[A-Za-z0-9_]+)-'
                     '(?P<compiler>[A-Za-z0-9_]+)-'
                     '(?P<model>[A-Za-z0-9_]+)-GPU-'
                     '(?P<cpu_or_gpu_value>[A-Za-z0-9_]+)-'
                     '(?P<arch>[A-Za-z0-9_]+)-'
                     '(?P<configuration>[A-Za-z0-9_]+)-'
                     'All(-(?P<extra_config>[A-Za-z0-9_]+)|)')
    keys = ['os', 'compiler', 'model', 'cpu_or_gpu_value', 'arch',
            'configuration', 'extra_config']
    with open(skia_directory + '/' + path) as f:
        jobs = json.load(f)
    def fmt(s):
        return s.encode('utf-8') if s is not None else ''
    rtype = collections.namedtuple('Job', keys)
    for job in jobs:
        m = reg.match(job)
        if m is not None:
            yield rtype(*(fmt(m.group(k)) for k in keys))

def gold_export_url(job, config, first_commit, last_commit):
    qq = [  ('source_type', 'gm'),
            ('config', config) ]
    for key in job._fields:
        qq.append( (key, job.__getattribute__(key)) )
    query = [
        ('fbegin', first_commit),
        ('fend',   last_commit),
        ('query',  urllib.urlencode(qq)),
        ('pos',    'true'),
        ('neg',    'false'),
        ('unt',    'false'),
        ('head',   'true')
    ]
    return 'https://public-gold.skia.org/json/export?' + urllib.urlencode(query)

def loc(tmpdir, i):
    return '%s/%04d.json' % (tmpdir, i)

def get_all_results(commit):
    all_results = {}
    all_results = collections.defaultdict(int)
    all_urls = []
    tmpdir = tempfile.mkdtemp(prefix='golder_')
    for job in get_jobs():
        CONFIGS = ['gles', 'vk']
        for config in CONFIGS:
            all_urls.append(gold_export_url(job, config, commit, commit))
    #for url in all_urls:
    #    print url
    threads = [
        threading.Thread(target=urllib.urlretrieve, args=(url, loc(tmpdir, i)))
            for i, url in enumerate(all_urls) ]
    for t in threads:
        t.start()
    for t in threads:
        t.join()

    for i, url in enumerate(all_urls):
        with open(loc(tmpdir, i)) as f:
             testResults = json.load(f)
        for t in testResults:
            # more than a single correct result for a config
            # still counts as one positive for these purposes
            all_results[t['testName']] += 1
            assert len(t['digests']) > 0
    shutil.rmtree(tmpdir)
    return all_results

def main(commit):
    all_results = get_all_results(commit)

    path = 'count_%s.json' % commit
    with open(path, 'w') as o:
        json.dump(all_results, o, indent=2, sort_keys=True)
    print path

def git_rev_parse(rev):
    return subprocess.check_output(['git', 'rev-parse', rev]).strip()

if __name__ == '__main__':
    if len(sys.argv) > 1:
        main(git_rev_parse(sys.argv[1]))
    else:
        sys.exit(1)
