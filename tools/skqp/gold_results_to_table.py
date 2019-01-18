#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import collections
import csv
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import urllib
import urllib2


def get_jobs():
    skia_directory = os.path.dirname(__file__) + '/../..'
    path = skia_directory + '/infra/bots/jobs.json'
    reg = re.compile('Test-(?P<os>[A-Za-z0-9_]+)-'
                     '(?P<compiler>[A-Za-z0-9_]+)-'
                     '(?P<model>[A-Za-z0-9_]+)-GPU-'
                     '(?P<cpu_or_gpu_value>[A-Za-z0-9_]+)-'
                     '(?P<arch>[A-Za-z0-9_]+)-'
                     '(?P<configuration>[A-Za-z0-9_]+)-'
                     'All(-(?P<extra_config>[A-Za-z0-9_]+)|)')
    keys = ['os', 'compiler', 'model', 'cpu_or_gpu_value', 'arch',
            'configuration', 'extra_config']
    def fmt(s):
        return s.encode('utf-8') if s is not None else ''
    with open(path) as f:
        jobs = json.load(f)
    for job in jobs:
        m = reg.match(job)
        if m is not None:
            yield [(k, fmt(m.group(k))) for k in keys]


def gold_export_url(job, config, first_commit, last_commit):
    qq = [ ('source_type', 'gm'), ('config', config) ] + job
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


def get_results_for_commit(commit, jobs):
    CONFIGS = ['gles', 'vk']
    list_of_passing_tests_for_each_job = {}
    def process(i, url):
        testResults = json.load(urllib2.urlopen(url))
        passing_tests = []
        for t in testResults:
            passing_tests.append(t['testName'])
            assert len(t['digests']) > 0
        list_of_passing_tests_for_each_job[i] = passing_tests
    all_urls = [gold_export_url(job, config, commit, commit)
                for job in jobs for config in CONFIGS]
    threads = [threading.Thread(target=process, args=(i, url))
               for i, url in enumerate(all_urls)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    all_results = collections.defaultdict(int)
    for passing_tests in list_of_passing_tests_for_each_job:
        for test in passing_tests:
            all_results[test] += 1
    return all_results


def git_rev_parse(rev):
    return subprocess.check_output(['git', 'rev-parse', rev]).strip()


def gold_results_to_table(N, output_path, starting_commit):
    jobs = [j for j in get_jobs()]
    all_results = collections.defaultdict(dict)
    for i in range(N):
        commit = starting_commit + '~%d' % i
        sys.stderr.write(commit + '\n')
        sys.stderr.flush()
        results = get_results_for_commit(git_rev_parse(commit), jobs)
        for test_name, count in results.items():
            all_results[test_name][i] = count

    with open(output_path, 'w') as o:
        w = csv.writer(o)
        keys = sorted(set(k for r in all_results.values() for k in r.keys()))
        w.writerow([''] + keys)
        for (test, test_result) in sorted(all_results.items()):
            w.writerow([test] + [test_result.get(k, 0) for k in keys])

if __name__ == '__main__':
    N = 40
    output_path = 'gold_results.csv'
    starting_commit = 'origin/master'
    gold_results_to_table(N, output_path, starting_commit)

