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
# Copy to new file, removing VulkanSamples
if not os.path.isfile(fetchDependenciesPath + 'fetchDependencies_nosamples'):
    with open(fetchDependenciesPath + 'fetchDependencies') as inFile:
        with open(fetchDependenciesPath + 'fetchDependencies_nosamples', "w") as outFile:
            for line in inFile:
                if "VulkanSamples" in line:
                    break
                outFile.write(line)
            outFile.write("# ----------------- Cleanup -------------------\n")
            outFile.write("\n")
            outFile.write("cd ..")
    os.chmod(fetchDependenciesPath + 'fetchDependencies_nosamples', 0o775)

p1 = subprocess.Popen('./fetchDependencies_nosamples', cwd=fetchDependenciesPath)
p1.wait()

# Build framework
print '************ build Framework ***************'
projPath = scriptDir + '/../externals/MoltenVK/MoltenVKPackaging.xcodeproj'
subprocess.check_call(['xcodebuild',
                       '-quiet',
                       '-project', projPath,
                       '-scheme', 'MoltenVK Package (Release) (' + osString + ' only)',
                       'build'])

# Copy framework to target out directory
frameworkPath = scriptDir + '/../externals/MoltenVK/MoltenVK/' + osString + '/MoltenVK.framework'
shutil.rmtree(outPath, True)
shutil.copytree(frameworkPath, outPath)
