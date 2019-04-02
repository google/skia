
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

def gni_sources_to_concated_c(gni_path, variable, dst):
    def parse_gni(gnifile):
        d = {}
        execfile(gnifile, d)
        return d 
    dst_dir = os.path.abspath(os.path.dirname(dst))
    sources = parse_gni(gni_path).get(variable)
    with open(dst, 'w') as o:
        opwd = os.getcwd()
        os.chdir(os.path.dirname(gni_path))
        for source in sources:
            rpath = os.path.relpath(source, dst_dir)
            o.write('#include "%s"\n' % rpath)
        os.chdir(opwd)

if __name__ == '__main__':
    gni_path, variable, dst = sys.argv[1:4]
    gni_sources_to_concated_c(gni_path, variable, dst)
