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

# sksl-minify uses the dependency list to ensure that minified names are unique at global scope.
dependencies = {
    'sksl_compute': ['sksl_gpu', 'sksl_shared'],
    'sksl_gpu': ['sksl_shared'],
    'sksl_frag': ['sksl_gpu', 'sksl_shared'],
    'sksl_vert': ['sksl_gpu', 'sksl_shared'],
    'sksl_graphite_frag': ['sksl_frag', 'sksl_gpu', 'sksl_shared'],
    'sksl_graphite_frag_es2': ['sksl_frag', 'sksl_gpu', 'sksl_shared'],
    'sksl_graphite_vert': ['sksl_vert', 'sksl_gpu', 'sksl_shared'],
    'sksl_graphite_vert_es2': ['sksl_vert', 'sksl_gpu', 'sksl_shared'],
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

        # Determine the program kind based on the module name.
        if "_compute" in module:
            programKind = "--compute"
        elif "_vert" in module:
            programKind = "--vert"
        else:
            programKind = "--frag"

        # Assemble the module dependency list and call sksl-minify to recreate the module in its
        # minified form.
        moduleList = [module]
        if moduleName not in dependencies:
            print("### Error compiling " + moduleName + ": dependency list must be specified")
            exit(1)
        for dependent in dependencies[moduleName]:
            moduleList.append(os.path.join(moduleDir, dependent) + ".sksl")

        # Generate fully-optimized and minified module data (for release/optimize-for-size builds).
        destPath = target + ".minified.sksl"
        args = [sksl_minify, programKind, "--stringify", destPath] + moduleList
        subprocess.check_output(args).decode('utf-8')

        # Generate unoptimized module data (used in debug, for improved readability).
        destPath = target + ".unoptimized.sksl"
        args = [sksl_minify, programKind, "--unoptimized", "--stringify", destPath] + moduleList
        subprocess.check_output(args).decode('utf-8')

    except subprocess.CalledProcessError as err:
        print("### Error compiling " + module + ":")
        print(err.output)
        exit(1)
