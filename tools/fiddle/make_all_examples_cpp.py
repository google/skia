#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
''' Run this script to re-generate the `all_examples.cpp` file after adding or
    deleting example fiddles. '''
import glob
import os
os.chdir(os.path.dirname(__file__))
with open('all_examples.cpp', 'w') as o:
    o.write('// Copyright 2019 Google LLC.\n// Use of this source code is '
            'governed by a BSD-style license that can be found in the '
            'LICENSE file.\n')
    for path in sorted(glob.glob('../../docs/examples/*.cpp')):
        # strip ../../
        path = path[6:]
        o.write('#include "%s"\n' % path)
