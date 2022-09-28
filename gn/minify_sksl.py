#!/usr/bin/env python
#
# Copyright 2020 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

sksl_minify = sys.argv[1]
targetDir = sys.argv[2]
modules = sys.argv[3:]

# This dependency list isn't currently referenced, but a more advanced minifier might need to know
# about dependent modules to ensure that names are always unique.
dependencies = {
    'sksl_compute': ['sksl_gpu', 'sksl_shared'],
    'sksl_gpu': ['sksl_shared'],
    'sksl_frag': ['sksl_gpu', 'sksl_shared'],
    'sksl_vert': ['sksl_gpu', 'sksl_shared'],
    'sksl_graphite_frag': ['sksl_frag', 'sksl_gpu', 'sksl_shared'],
    'sksl_graphite_vert': ['sksl_vert', 'sksl_gpu', 'sksl_shared'],
    'sksl_public': ['sksl_shared'],
    'sksl_rt_shader': ['sksl_public', 'sksl_shared'],
    'sksl_shared': [],
}

for module in modules:
    try:
        moduleDir, moduleName = os.path.split(os.path.splitext(module)[0])
        if not os.path.isdir(targetDir):
            os.mkdir(targetDir)
        target = os.path.join(targetDir, moduleName)

        # Call sksl-minify to recreate the module without whitespace and comments.
        args = [sksl_minify, target + ".minified.sksl", module]
        subprocess.check_output(args).decode('utf-8')

    except subprocess.CalledProcessError as err:
        print("### Error compiling " + module + ":")
        print(err.output)
        exit(1)
