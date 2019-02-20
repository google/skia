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
osDir = sys.argv[2]

# Run fetchDependencies
fetchDependenciesPath = scriptDir + '/../externals/MoltenVK/fetchDependencies'
subprocess.check_call([fetchDependenciesPath])

# Build framework
projPath = scriptDir + '/../externals/MoltenVK/MoltenVKPackaging.xcodeproj'
subprocess.check_call(['xcodebuild',
                       '-quiet',
                       '-project', projPath,
                       '-scheme', 'MoltenVK (Release)',
                       'build'])

# Copy framework to target out directory
frameworkPath = scriptDir + '/../externals/MoltenVK/MoltenVK/' + osDir + '/MoltenVK.framework'
shutil.rmtree(outPath, True)
shutil.copytree(frameworkPath, outPath)
