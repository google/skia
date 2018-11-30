#! /usr/bin/env python

# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from subprocess import check_output
import os
import re
import sys
import tempfile

bucket = 'skia-skqp'

assert '/' in [os.sep, os.altsep]
assert '..' == os.pardir

skia_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))

cmd = ['gsutil', 'ls', 'gs://' + bucket]
extant = set(l.strip() for l in check_output(cmd).split('\n') if l)

header = '''<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>SkQP Pre-built APKs</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
body {
font-family:sans-serif;
max-width:55em;
margin:8px auto;
padding:0 8px;
}
td { padding:12px 6px; }
</style>
</head>
<body>
<h1>SkQP Pre-built APKs</h1>
'''
footer = '</body>\n</html>\n'

def find(short, extant):
    key_fmt = 'gs://%s/skqp-universal-%s.apk'
    while len(short) > 8:
        key = key_fmt % (bucket, short)
        if key in extant:
            return key
        short = short[:-1]
    return None

def table(o, from_commit, to_commit):
    env_copy = os.environ.copy()
    env_copy['TZ'] = ''
    o.write('<h2>%s %s</h2>\n' % (to_commit, ' '.join(from_commit)))
    o.write('<table>\n<tr><th>APK</th><th>Date</th><th>Commit</th></tr>\n')
    cmd = ['git', 'log'] + from_commit + [to_commit, '--format=%H']
    for line in check_output(cmd).split('\n'):
        commit = line.strip()
        if not commit:
            continue
        short = check_output(['git', 'log', '-1',
                              '--format=%h', commit]).strip()
        name = find(short, extant)

        if name is not None:
            url = re.sub('gs://', 'https://storage.googleapis.com/', name)
            apk_name = 'skqp-universal-%s.apk' % short
        else:
            url = ''
            apk_name = ''
        date = check_output(['git', 'log', '-1', commit,
                             '--date=format-local:%Y-%m-%d %H:%M:%S %Z',
                             '--format=%cd'], env=env_copy).strip()
        subj = check_output(['git', 'log', '-1', commit,
                             '--format=%<(50,trunc)%s']).strip()
        commit_url = 'https://skia.googlesource.com/skia/+/' + commit

        o.write('<tr>\n<td><a href="%s">%s</a></td>\n'
                '<td>%s</td>\n<td><a href="%s">%s</a></td>\n</tr>\n' %
                (url, apk_name, date, commit_url, subj))
    o.write('</table>\n')

if __name__ == '__main__':
    d = tempfile.mkdtemp()
    path = os.path.join(d, 'apklist.html')
    with open(path, 'w') as o:
        o.write(header)
        table(o, ['^origin/master', '^3e34285f2a0'], 'origin/skqp/dev')
        table(o, ['^origin/master'], 'origin/skqp/release')
        o.write(footer)
    print path
    cmd = 'gsutil -h "Content-Type:text/html" cp "%s" gs://skia-skqp/apklist'
    print cmd % path




