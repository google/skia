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

stats = {}

for filename in os.listdir(folder):
    basename, ext = os.path.splitext(filename)
    if ext not in ['.frag', '.spv']:
        continue
    cmdline = [compiler]
    if ext == '.spv':
        cmdline.extend(['-f', '-p'])
    cmdline.append(os.path.join(folder, filename))
    try:
        output = subprocess.check_output(cmdline)
    except subprocess.CalledProcessError:
        continue
    stats.setdefault(basename, {})
    for line in output.splitlines():
        if line.startswith('Instructions Emitted'):
            inst = line.split(':')[1].split()
            stats[basename][ext] = inst

for k, v in stats.iteritems():
    gl = v.get('.frag', ['', '', ''])
    vk = v.get('.spv', ['', '', ''])
    print '{0},{1},{2},{3},{4},{5},{6}'.format(k, gl[0], gl[1], gl[2], vk[0], vk[1], vk[2])
