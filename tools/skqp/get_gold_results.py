#! /usr/bin/env python

# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys
import time
import urllib

def gold_export_url(first_commit, last_commit):
    query = [
        ('fbegin', first_commit),
        ('fend',   last_commit),
        ('query',  'config=gles&config=vk&source_type=gm'),
        ('pos',    'true'),
        ('neg',    'false'),
        ('unt',    'false')
    ]
    return 'https://public-gold.skia.org/json/export?' + urllib.urlencode(query)

def git_rev_parse(rev):
    return subprocess.check_output(['git', 'rev-parse', rev]).strip()

def main(args):
    if len(args) != 2:
        sys.stderr.write('Usage:\n  %s FIRST_COMMIT LAST_COMMIT\n' % __file__)
        sys.exit(1)
    c1 = git_rev_parse(args[0])
    c2 = git_rev_parse(args[1])
    now = time.strftime("%Y%m%d_%H%M%S", time.gmtime())
    url = gold_export_url(c1, c2)
    sys.stdout.write(url + '\n')
    filename = 'meta_%s_%s_%s.json' % (now, c1[:16], c2[:16])
    urllib.urlretrieve(url, filename)
    sys.stdout.write('\n' + filename + '\n')

if __name__ == '__main__':
    main(sys.argv[1:])

