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

oldCwd = os.getcwd()

scriptDir = os.path.dirname(os.path.realpath(__file__))
outPath = oldCwd + '/' + sys.argv[1] + '/MoltenVK.framework'
osString = sys.argv[2]

moltenVKSrcPath = scriptDir + '/../externals/MoltenVK'
os.chdir(moltenVKSrcPath)

# Run fetchDependencies
print '************ fetchDependencies ***************'
p1 = subprocess.Popen('./fetchDependencies')
p1.wait()

dirs = os.listdir('External/glslang/build/External/spirv-tools')
for file in dirs:
   print file

# Build framework
print '************ build Framework ***************'
print 'Script Dir: ' + scriptDir
print 'CWD: ' + os.getcwd()
subprocess.check_call(['xcodebuild',
                       '-quiet',
                       '-project', 'MoltenVKPackaging.xcodeproj',
                       '-scheme', 'MoltenVK Package (' + osString + ' only)',
                       'build'])

# Copy framework to target out directory
frameworkPath = 'MoltenVK/' + osString + '/framework/MoltenVK.framework'
print frameworkPath
print outPath
shutil.rmtree(outPath, True)
shutil.copytree(frameworkPath, outPath)

os.chdir(oldCwd)
