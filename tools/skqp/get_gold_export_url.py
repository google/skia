#! /usr/bin/env python

# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys
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
    return 'https://gold.skia.org/json/export?' + urllib.urlencode(query)

def git_rev_parse(rev):
    return subprocess.check_output(['git', 'rev-parse', rev]).strip()

def main(args):
    if len(args) != 2:
        sys.stderr.write('Usage:\n  %s FIRST_COMMIT LAST_COMMIT\n' % __file__)
        sys.exit(1)
    sys.stdout.write(gold_export_url(git_rev_parse(args[0]),
                                     git_rev_parse(args[1])) + '\n')

if __name__ == '__main__':
    main(sys.argv[1:])

