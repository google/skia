#! /usr/bin/env python

# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from subprocess import check_output, CalledProcessError
import os
import re
import sys
import tempfile

HEADER = '''<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>SkQP Pre-built APKs</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
body {
font-family:sans-serif;
max-width:50em;
margin:8px auto;
padding:0 8px;
}
table { max-width:100%; border-collapse: collapse; }
td { padding:12px 8px; vertical-align:top; }
tr:nth-child(even) {background: #F2F2F2; color:#000;}
tr:nth-child(odd)  {background: #FFFFFF; color:#000;}
</style>
</head>
<body>
<h1>SkQP Pre-built APKs</h1>
'''
FOOTER = '</body>\n</html>\n'

BUCKET = 'skia-skqp'

NAME_FMT = 'skqp-universal-%s.apk'

def get_existing_files():
    cmd = ['gsutil', 'ls', 'gs://' + BUCKET]
    try:
        output = check_output(cmd)
    except (OSError, CalledProcessError):
        sys.stderr.write('command: "%s" failed.\n' % ' '.join(cmd))
        sys.exit(1)
    result = set()
    regex = re.compile('gs://%s/%s' % (BUCKET, NAME_FMT % '([0-9a-f]+)'))
    for line in output.split('\n'):
        m = regex.match(line.strip())
        if m is not None:
            result.add(m.group(1))
    return result

def find(v, extant):
    l = min(16, len(v))
    while l > 8:
        if v[:l] in extant:
            return v[:l]
        l -= 1
    return None

def nowrap(s):
    return (s.replace(' ', u'\u00A0'.encode('utf-8'))
             .replace('-', u'\u2011'.encode('utf-8')))

def table(o, from_commit, to_commit):
    env_copy = os.environ.copy()
    env_copy['TZ'] = ''
    extant = get_existing_files()
    o.write('<h2>%s %s</h2>\n' % (to_commit, ' '.join(from_commit)))
    o.write('<table>\n<tr><th>APK</th><th>Date</th><th>Commit</th></tr>\n')
    git_cmd = ['git', 'log', '--format=%H;%cd;%<(100,trunc)%s',
               '--date=format-local:%Y-%m-%d %H:%M:%S %Z'
               ] + from_commit + [to_commit]
    commits = check_output(git_cmd, env=env_copy)
    for line in commits.split('\n'):
        line = line.strip()
        if not line:
            continue
        commit, date, subj = line.split(';', 2)
        short = find(commit, extant)
        if short is not None:
            apk_name =  NAME_FMT % short
            url = 'https://storage.googleapis.com/%s/%s' % (BUCKET, apk_name)
        else:
            apk_name, url =  '', ''
        commit_url = 'https://skia.googlesource.com/skia/+/' + commit
        o.write('<tr>\n<td><a href="%s">%s</a></td>\n'
                '<td>%s</td>\n<td><a href="%s">%s</a></td>\n</tr>\n' %
                (url, nowrap(apk_name), nowrap(date), commit_url, subj))
    o.write('</table>\n')

def main():
    assert '/' in [os.sep, os.altsep] and '..' == os.pardir
    os.chdir(os.path.join(os.path.dirname(__file__), '../..'))
    d = tempfile.mkdtemp()
    path = os.path.join(d, 'apklist.html')
    with open(path, 'w') as o:
        o.write(HEADER)
        table(o, ['^origin/master', '^3e34285f2a0'], 'origin/skqp/dev')
        table(o, ['^origin/master'], 'origin/skqp/release')
        o.write(FOOTER)
    print path
    gscmd = 'gsutil -h "Content-Type:text/html" cp "%s" gs://skia-skqp/apklist'
    print gscmd % path

if __name__ == '__main__':
    main()

