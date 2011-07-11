#    Copyright 2011 Google Inc.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

# "Makefile" replacement to build skia for Windows.
# More info at http://code.google.com/p/skia/wiki/DocRoot

import os
import shutil
import sys

# TODO(epoger): add special 'all' target
TARGET_CLEAN = 'clean'

SCRIPT_DIR = os.path.abspath(os.path.dirname(__file__))
OUT_SUBDIR = 'out'
GYP_SUBDIR = 'gyp'


# Simple functions that report what they are doing, and exit(1) on failure.
def cd(path):
    print '> cd %s' % path
    if not os.path.exists(path):
        print 'path %s does not exist' % path
        sys.exit(1)
    os.chdir(path)

def rmtree(path):
    print '> rmtree %s' % path
    shutil.rmtree(path, ignore_errors=True)

def runcommand(command):
    print '> %s' % command
    if os.system(command):
        sys.exit(1)

def MakeClean():
    """Cross-platform "make clean" operation."""
    cd(SCRIPT_DIR)
    rmtree(OUT_SUBDIR)
    # clean up the directory that XCode (on Mac) creates
    rmtree('xcodebuild')


def CheckWindowsEnvironment():
    """For Windows: check environment variables needed for command-line build.

    If those environment variables are missing, try to set them.
    If environment variables can be set up, this function returns; otherwise,
    it displays an error message and exits.
    """
    # If we already have the proper environment variables, nothing to do here.
    try:
        env_DevEnvDir = os.environ['DevEnvDir']
        return  # found it, so we are done
    except KeyError:
        pass # go on and run the rest of this function

    print ('\nCould not find Visual Studio environment variables.'
           '\nPerhaps you have not yet run vcvars32.bat as described at'
           '\nhttp://msdn.microsoft.com/en-us/library/f2ccy3wt.aspx ?')
    found_path = None
    try:
        possible_path = os.path.abspath(os.path.join(
            os.environ['VS100COMNTOOLS'], os.path.pardir, os.path.pardir,
            'VC', 'bin', 'vcvars32.bat'))
        if os.path.exists(possible_path):
            found_path = possible_path
    except KeyError:
        pass
    if found_path:
        print '\nIt looks like you can run that script at:\n%s' % found_path
    else:
        print '\nUnable to find vcvars32.bat on your system.'
    sys.exit(1)


def MakeWindows(args):
    """For Windows: build as appropriate for the command line arguments.

    parameters:
        args: command line arguments as a list of strings
    """
    CheckWindowsEnvironment()
    # TODO(epoger): what about parameters?  (fixed vs float, debug vs release)
    # TODO(epoger): what about "make" flags (like -j) that Windows doesn't support?

    # Run gyp_skia to prepare Visual Studio projects.
    cd(SCRIPT_DIR)
    runcommand('python gyp_skia')
    
    for target in args:
        # Check for special-case targets
        if target == TARGET_CLEAN:
            MakeClean()
        else:
            cd(os.path.join(SCRIPT_DIR, OUT_SUBDIR, GYP_SUBDIR))
            runcommand('msbuild /target:%s %s.sln' % (target, target))


# main:
# dispatch to appropriate Make<Platform>() variant.
if os.name == 'nt':
    MakeWindows(sys.argv[1:])
    sys.exit(0)
elif os.name == 'posix':
    if sys.platform == 'darwin':
        print 'Mac developers should not run this script; see ' \
            'http://code.google.com/p/skia/wiki/GettingStartedOnMac'
        sys.exit(1)
    else:
        print 'Unix developers should not run this script; see ' \
            'http://code.google.com/p/skia/wiki/GettingStartedOnLinux'
        sys.exit(1)
else:
    print 'unknown platform (os.name=%s, sys.platform=%s); see %s' % (
        os.name, sys.platform, 'http://code.google.com/p/skia/wiki/DocRoot')
    sys.exit(1)
sys.exit(0)
    

