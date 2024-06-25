#!/usr/bin/env python3
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Run this script to re-generate the `all_examples.cpp` file after adding or
deleting example fiddles.
"""
import argparse
import difflib
import glob
import os

ALL_EXAMPLES = 'all_examples.cpp'


parser = argparse.ArgumentParser()
parser.add_argument(
    '--print-diff',
    action='store_true',
    help='Print the diff between the old and new `all_examples.cpp` file.',
)
args = parser.parse_args()


os.chdir(os.path.dirname(__file__))

with open(ALL_EXAMPLES, 'r') as o:
    prev_lines = o.readlines()

with open(ALL_EXAMPLES, 'w+') as o:
    o.write(
        '// Copyright 2019 Google LLC.\n// Use of this source code is '
        'governed by a BSD-style license that can be found in the '
        'LICENSE file.\n'
    )
    for path in sorted(glob.glob('../../docs/examples/*.cpp')):
        # strip ../../
        path = path[6:]
        o.write('#include "%s"\n' % path)

if args.print_diff:
    with open(ALL_EXAMPLES, 'r') as o:
        diff = difflib.unified_diff(prev_lines, o.readlines())
        print(''.join(diff))
