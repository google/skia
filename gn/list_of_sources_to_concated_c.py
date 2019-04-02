#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import json
import os
import sys

def list_of_sources_to_concated_c(path, dst):
    dst_dir = os.path.abspath(os.path.dirname(dst))
    with open(path, 'r') as f:
        sources = json.load(f)
    with open(dst, 'w') as o:
        opwd = os.getcwd()
        os.chdir(os.path.dirname(path))
        for source in sources:
            rpath = os.path.relpath(source, dst_dir)
            o.write('#include "%s"\n' % rpath)
        os.chdir(opwd)

if __name__ == '__main__':
    list_of_sources_to_concated_c(sys.argv[1], sys.argv[2])
