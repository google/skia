# Copyright 2017 Google Inc.
# 
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import re
import subprocess
import sys

endings = re.compile(r'.*(\.cpp|\.h|\.c|\.hh)$')

os.chdir(os.path.join(os.path.dirname(__file__), os.pardir, os.pardir))

def all_ascii(path):
    with open(path, 'rb') as f:
        while True:
            buff = f.read(4096)
            if 0 == len(buff):
                return True
            if any(ord(c) > 128 for c in buff):
                return False

for path in subprocess.check_output(['git', 'ls-files']).split('\n'):
    if not endings.match(path) or all_ascii(path):
        continue
    print path
    with open(path, 'rb') as f:
        contents = f.read()
    with open(path, 'wb') as o:
        o.write('\xEF\xBB\xBF /* byte order mark */\n')
        o.write(contents)

