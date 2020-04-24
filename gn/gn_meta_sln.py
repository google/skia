# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import glob
import re
import sys
from shutil import copyfile

# Helpers
def ensureExists(path):
    try:
        os.makedirs(path)
    except OSError:
        pass

def writeLinesToFile(lines, fileName):
    ensureExists(os.path.dirname(fileName))
    with open(fileName, "w") as f:
        f.writelines(lines)

def extractIdg(projFileName):
    result = []
    with open(projFileName) as projFile:
        lines = iter(projFile)
        for pLine in lines:
            if "<ItemDefinitionGroup" in pLine:
                while not "</ItemDefinitionGroup" in pLine:
                    result.append(pLine)
                    pLine = lines.next()
                result.append(pLine)
                return result

# [ (name, hasSln), ... ]
configs = []

# Find all directories that can be used as configs (and record if they have VS
# files present)
for root, dirs, files in os.walk("out"):
    for outDir in dirs:
        gnFile = os.path.join("out", outDir, "build.ninja.d")
        if os.path.exists(gnFile):
            slnFile = os.path.join("out", outDir, "all.sln")
            configs.append((outDir, os.path.exists(slnFile)))
    break

# Every project has a GUID that encodes the type. We only care about C++.
cppTypeGuid = "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942"

# name -> [ (config, pathToProject, GUID), ... ]
allProjects = {}
projectPattern = (r'Project\("\{' + cppTypeGuid +
                  r'\}"\) = "([^"]*)", "([^"]*)", "\{([^\}]*)\}"')
projectNamePattern = (r'obj/(.*)\.vcxproj')

for config in configs:
    if config[1]:
        slnLines = iter(open("out/" + config[0] + "/all.sln"))
        for slnLine in slnLines:
            matchObj = re.match(projectPattern, slnLine)
            if matchObj:
                projPath = matchObj.group(2)
                nameObj = re.match(projectNamePattern, projPath)
                if nameObj:
                    projName = nameObj.group(1).replace('/', '.')
                    if not allProjects.has_key(projName):
                        allProjects[projName] = []
                    allProjects[projName].append((config[0], projPath,
                                                 matchObj.group(3)))

# We need something to work with. Typically, this will fail if no GN folders
# have IDE files
if len(allProjects) == 0:
    print "ERROR: At least one GN directory must have been built with --ide=vs"
    sys.exit()

# Create a new solution. We arbitrarily use the first config as the GUID source
# (but we need to match that behavior later, when we copy/generate the project
# files).
newSlnLines = []
newSlnLines.append(
    'Microsoft Visual Studio Solution File, Format Version 12.00\n')
newSlnLines.append('# Visual Studio 2015\n')
for projName, projConfigs in allProjects.items():
    newSlnLines.append('Project("{' + cppTypeGuid + '}") = "' + projName +
                       '", "' + projConfigs[0][1] + '", "{' + projConfigs[0][2]
                       + '}"\n')
    newSlnLines.append('EndProject\n')

newSlnLines.append('Global\n')
newSlnLines.append(
    '\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n')
for config in configs:
    newSlnLines.append('\t\t' + config[0] + '|x64 = ' + config[0] + '|x64\n')
newSlnLines.append('\tEndGlobalSection\n')
newSlnLines.append(
    '\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n')
for projName, projConfigs in allProjects.items():
    projGuid = projConfigs[0][2]
    for config in configs:
        newSlnLines.append('\t\t{' + projGuid + '}.' + config[0] +
                           '|x64.ActiveCfg = ' + config[0] + '|x64\n')
        newSlnLines.append('\t\t{' + projGuid + '}.' + config[0] +
                           '|x64.Build.0 = ' + config[0] + '|x64\n')
newSlnLines.append('\tEndGlobalSection\n')
newSlnLines.append('\tGlobalSection(SolutionProperties) = preSolution\n')
newSlnLines.append('\t\tHideSolutionNode = FALSE\n')
newSlnLines.append('\tEndGlobalSection\n')
newSlnLines.append('\tGlobalSection(NestedProjects) = preSolution\n')
newSlnLines.append('\tEndGlobalSection\n')
newSlnLines.append('EndGlobal\n')

# Write solution file
writeLinesToFile(newSlnLines, "out/sln/skia.sln")

idgHdr = "<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='"

# Now, bring over the project files
for projName, projConfigs in allProjects.items():
    # Paths to project and filter file in src and dst locations
    srcProjPath = os.path.join("out", projConfigs[0][0], projConfigs[0][1])
    dstProjPath = os.path.join("out", "sln", projConfigs[0][1])
    srcFilterPath = srcProjPath + ".filters"
    dstFilterPath = dstProjPath + ".filters"

    # Copy the filter file unmodified
    ensureExists(os.path.dirname(dstProjPath))
    copyfile(srcFilterPath, dstFilterPath)

    # Bring over the project file, modified with extra configs
    with open(srcProjPath) as srcProjFile:
        projLines = iter(srcProjFile)
        newProjLines = []
        for line in projLines:
            if "<ItemDefinitionGroup" in line:
                # This is a large group that contains many settings. We need to
                # replicate it, with conditions so it varies per configuration.
                idgLines = []
                while not "</ItemDefinitionGroup" in line:
                    idgLines.append(line)
                    line = projLines.next()
                idgLines.append(line)
                for projConfig in projConfigs:
                    configIdgLines = extractIdg(os.path.join("out",
                                                             projConfig[0],
                                                             projConfig[1]))
                    newProjLines.append(idgHdr + projConfig[0] + "|x64'\">\n")
                    for idgLine in configIdgLines[1:]:
                        newProjLines.append(idgLine)
            elif "ProjectConfigurations" in line:
                newProjLines.append(line)
                projConfigLines = [
                    projLines.next(),
                    projLines.next(),
                    projLines.next(),
                    projLines.next() ]
                for config in configs:
                    for projConfigLine in projConfigLines:
                        newProjLines.append(projConfigLine.replace("GN",
                                                                   config[0]))
            elif "<OutDir" in line:
                newProjLines.append(line.replace(projConfigs[0][0],
                                                 "$(Configuration)"))
            else:
                newProjLines.append(line)
        with open(dstProjPath, "w") as newProj:
            newProj.writelines(newProjLines)
