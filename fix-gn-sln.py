# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import glob
import sys
from shutil import copyfile

# Get list of existing directories to use as configs
configs = []
configsWithSln = []
srcDir = ""
newestSlnTimestamp = 0
for root, dirs, files in os.walk("out"):
    for outDir in dirs:
        gnFile = os.path.join("out", outDir, "build.ninja.d")
        slnFile = os.path.join("out", outDir, "all.sln")
        if os.path.exists(gnFile):
            configs.append(outDir)
        if os.path.exists(slnFile):
            configsWithSln.append(outDir)
            slnTimestamp = os.path.getmtime(slnFile)
            if slnTimestamp > newestSlnTimestamp:
                newestSlnTimestamp = slnTimestamp
                srcDir = outDir
    break

# We need at least one config with a solution
if len(configsWithSln) == 0:
    print "ERROR: At least one GN directory must have been built with --ide=vs"
    sys.exit()

# Ensure directories exist
try:
    os.makedirs("out/sln/obj")
except OSError:
    pass

# Copy filter files unmodified
for filterFile in glob.glob("out/" + srcDir + "/obj/*.filters"):
    copyfile(filterFile, filterFile.replace("out/" + srcDir, "out/sln"))

# Copy Solution file, with additional configurations
slnLines = iter(open("out/" + srcDir + "/all.sln"))
newSlnLines = []

for line in slnLines:
    newSlnLines.append(line)
    if "SolutionConfigurationPlatforms" in line:
        slnConfig = slnLines.next()
        for config in configs:
            newSlnLines.append(slnConfig.replace("GN", config))
    elif "ProjectConfigurationPlatforms" in line:
        activeCfg = slnLines.next()
        while "EndGlobalSection" not in activeCfg:
            buildCfg = slnLines.next()
            for config in configs:
                newSlnLines.append(activeCfg.replace("GN", config))
                newSlnLines.append(buildCfg.replace("GN", config))
            activeCfg = slnLines.next()
        newSlnLines.append(activeCfg)

with open("out/sln/skia.sln", "w") as newSln:
    newSln.writelines(newSlnLines)

# Now bring over all project files with modification
for srcProjFilename in glob.glob("out/" + srcDir + "/obj/*.vcxproj"):
    with open(srcProjFilename) as srcProjFile:
        projLines = iter(srcProjFile)
        newProjLines = []
        for line in projLines:
            if "ProjectConfigurations" in line:
                newProjLines.append(line)
                projConfigLines = [
                    projLines.next(),
                    projLines.next(),
                    projLines.next(),
                    projLines.next() ]
                for config in configs:
                    for projConfigLine in projConfigLines:
                        newProjLines.append(
                            projConfigLine.replace("GN", config))
            elif "<OutDir" in line:
                newProjLines.append(line.replace(srcDir, "$(Configuration)"))
            else:
                newProjLines.append(line)
        newName = "out/sln/obj/" + os.path.basename(srcProjFilename)
        with open(newName, "w") as newProj:
            newProj.writelines(newProjLines)
