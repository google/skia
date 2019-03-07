#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import glob
import os
head = '''# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
'''
def gni(path, name, files):
    with open(path, 'w') as o:
        o.write('%s\n%s = get_path_info([\n' % (head, name))
        for x in sorted(files):
            o.write('  "%s",\n' % x)
        o.write('], "abspath")\n')
if __name__ == '__main__':
    os.chdir(os.path.dirname(__file__))
    gni('examples.gni', 'examples_sources', glob.glob('../docs/examples/*.cpp'))
