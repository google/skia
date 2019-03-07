#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import os, glob
os.chdir(os.path.dirname(__file__))
def f(filename, name, globber):
    with open(filename, 'w') as o:
        o.write('# Copyright 2019 Google LLC.\n')
        o.write('# Use of this source code is governed by a BSD-style license that can be\n')
        o.write('# found in the LICENSE file.\n')
        o.write('%s = get_path_info([\n' % name)
        for x in sorted(glob.glob(globber)):
            o.write('  "%s",\n' % x)
        o.write('], "abspath")\n')
f('examples.gni', 'examples_sources', '../examples/*.cpp')
