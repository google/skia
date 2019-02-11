#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import os
import re
import subprocess
import sys
import threading
import urllib
import urllib2


assert '/' in [os.sep, os.altsep]


skia_directory = os.path.abspath(os.path.dirname(__file__) + '/../..')


def get_jobs():
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
    qq = [('source_type', 'gm'), ('config', config)] + job
    query = [
        ('fbegin', first_commit),
        ('fend', last_commit),
        ('query', urllib.urlencode(qq)),
        ('pos', 'true'),
        ('neg', 'false'),
        ('unt', 'false'),
        ('head', 'true')
    ]
    return 'https://public-gold.skia.org/json/export?' + urllib.urlencode(query)


def get_results_for_commit(commit, jobs):
    sys.stderr.write('%s\n' % commit)
    sys.stderr.flush()
    CONFIGS = ['gles', 'vk']
    passing_tests_for_all_jobs = []
    def process(url):
        testResults = json.load(urllib2.urlopen(url))
        sys.stderr.write('.')
        sys.stderr.flush()
        passing_tests = 0
        for t in testResults:
            assert t['digests']
            passing_tests += 1
        passing_tests_for_all_jobs.append(passing_tests)
    all_urls = [gold_export_url(job, config, commit, commit)
                for job in jobs for config in CONFIGS]
    threads = [threading.Thread(target=process, args=(url,)) for url in all_urls]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    result = sum(passing_tests_for_all_jobs)
    sys.stderr.write('\n%d\n' % result)
    sys.stderr.flush()
    return result


def find_best_commit(commits):
    jobs = [j for j in get_jobs()]
    results = []
    for commit_name in commits:
        commit_hash = subprocess.check_output(['git', 'rev-parse', commit_name]).strip()
        results.append((commit_hash, get_results_for_commit(commit_hash, jobs)))

    best_result = max(r for h, r in results)
    for h, r in results:
        if r == best_result:
            return h
    return None


def generate_commit_list(args):
    return subprocess.check_output(['git', 'log', '--format=%H'] + args).splitlines()


def main(args):
    os.chdir(skia_directory)
    subprocess.check_call(['git', 'fetch', 'origin'])
    sys.stderr.write('%s\n' % ' '.join(args))
    commits = generate_commit_list(args)
    sys.stderr.write('%d\n' % len(commits))
    best = find_best_commit(commits)
    sys.stderr.write('DONE:\n')
    sys.stderr.flush()
    sys.stdout.write('%s\n' % best)


usage = '''Example usage:
    python %s origin/master ^origin/skqp/dev < /dev/null > LOG 2>&1 & disown
'''

if __name__ == '__main__':
    if len(sys.argv) < 2:
        sys.stderr.write(usage % sys.argv[0])
        sys.exit(1)
    main(sys.argv[1:])
