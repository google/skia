# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import os
import subprocess
import sys

if len(sys.argv) != 3:
    print sys.argv[0], ' <compiler> <folder>'
    sys.exit(1)

compiler = sys.argv[1]
folder = sys.argv[2]

FRAG_JSON = os.path.join(folder, 'frag.json')

with open(FRAG_JSON) as f:
    frags = json.load(f)

if not frags:
    print 'No JSON data'
    sys.exit(1)

for fragAndCount in frags:
    source = os.path.join(folder, fragAndCount[0] + '.frag')
    try:
        output = subprocess.check_output([compiler, source])
    except subprocess.CalledProcessError:
        continue
    for line in output.splitlines():
        if line.startswith('Instructions Emitted'):
            inst = line.split(':')[1].split()
            print '{0} {1} {2} {3} {4}'.format(
                fragAndCount[0], fragAndCount[1], inst[0], inst[1], inst[2])
