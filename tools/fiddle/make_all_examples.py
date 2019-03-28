#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import glob
import os
head = '''// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'''
if __name__ == '__main__':
    os.chdir(os.path.dirname(__file__))
    with open('all_examples.cpp', 'w') as o:
        o.write(head)
        for path in sorted(glob.glob('../../docs/examples/*.cpp')):
            o.write('#include "%s"\n' % path)
