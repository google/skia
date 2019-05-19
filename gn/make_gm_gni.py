#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import os
import glob
os.chdir(os.path.dirname(__file__))
with open('gm.gni', 'w') as o:
    o.write('''# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Things are easiest for everyone if these source paths are absolute.
_gm = get_path_info("../gm", "abspath")

gm_sources = [
''')
    for path in sorted(glob.glob('../gm/*.c*')):
        o.write('  "%s",\n' % path.replace('../gm', '$_gm'))
    o.write(']\n')
