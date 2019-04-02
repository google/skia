#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re
import os
import sys
import urllib2

def check_url_status(url):
    try:
        f = urllib2.urlopen(url)
    except urllib2.HTTPError:
        return False
    stat = f.getcode()
    f.close()
    return stat == 200

def find_files(path):
    for d, _, fs in os.walk(path):
        for f in fs:
            yield os.path.join(d, f)

def main():
    good = True
    regex = re.compile(r'\bhttps?://\S+')
    spath = os.path.dirname(__file__) + '/../include'
    for path in find_files(spath):
        sys.stderr.write('.')
        with open(path, 'r') as f:
            content = f.read()
        for url in regex.findall(content):
            if not check_url_status(url):
                sys.stdout.write('\n%s %s\n' % (path, url))
                good = False
    sys.stderr.write('\n')
    return good

if __name__ == '__main__':
    sys.exit(0 if main() else 1)
