#!/usr/bin/env python2.7
#
# Copyright 2019 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import shutil
import subprocess
import sys

scriptDir = os.path.dirname(os.path.realpath(__file__))
outPath = sys.argv[1] + '/MoltenVK.framework'
osString = sys.argv[2]

# Run fetchDependencies
print '************ fetchDependencies ***************'
fetchDependenciesPath = scriptDir + '/../externals/MoltenVK/'
p1 = subprocess.Popen('./fetchDependencies', cwd=fetchDependenciesPath)
p1.wait()

# Build framework
print '************ build Framework ***************'
projPath = scriptDir + '/../externals/MoltenVK/MoltenVKPackaging.xcodeproj'
subprocess.check_call(['xcodebuild',
                       '-quiet',
                       '-project', projPath,
                       '-scheme', 'MoltenVK Package (' + osString + ' only)',
                       'build'])

# Copy framework to target out directory
frameworkPath = scriptDir + '/../externals/MoltenVK/MoltenVK/' + osString + '/framework/MoltenVK.framework'
shutil.rmtree(outPath, True)
shutil.copytree(frameworkPath, outPath)
