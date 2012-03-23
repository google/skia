# Copyright 2011 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# "Makefile" replacement to build skia for Windows.
# More info at http://code.google.com/p/skia/wiki/DocRoot
#
# Some usage examples:
#   make clean
#   make tests
#   make bench BUILDTYPE=Release
#   make gm GYP_DEFINES=skia_scalar=fixed BUILDTYPE=Release
#   make all

import os
import shutil
import sys

BUILDTYPE = 'Debug'

# special targets
TARGET_ALL = 'all'
TARGET_CLEAN = 'clean'
TARGET_GYP = 'gyp'
LIST_OF_ALL_TARGETS = ['SampleApp', 'bench', 'gm', 'tests', 'tools']

SCRIPT_DIR = os.path.abspath(os.path.dirname(__file__))
OUT_SUBDIR = 'out'
GYP_SUBDIR = 'gyp'


# Simple functions that report what they are doing, and exit(1) on failure.
def cd(path):
    print '> cd %s' % path
    if not os.path.isdir(path):
        print 'directory %s does not exist' % path
        sys.exit(1)
    os.chdir(path)

def rmtree(path):
    print '> rmtree %s' % path
    shutil.rmtree(path, ignore_errors=True)

def mkdirs(path):
    print '> mkdirs %s' % path
    if not os.path.isdir(path):
        os.makedirs(path)

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


def MakeWindows(targets):
    """For Windows: build as appropriate for the command line arguments.

    parameters:
        targets: build targets as a list of strings
    """
    CheckWindowsEnvironment()

    # Run gyp_skia to prepare Visual Studio projects.
    cd(SCRIPT_DIR)
    runcommand('python gyp_skia')

    # Prepare final output dir into which we will copy all binaries.
    msbuild_working_dir = os.path.join(SCRIPT_DIR, OUT_SUBDIR, GYP_SUBDIR)
    msbuild_output_dir = os.path.join(msbuild_working_dir, BUILDTYPE)
    final_output_dir = os.path.join(SCRIPT_DIR, OUT_SUBDIR, BUILDTYPE)
    mkdirs(final_output_dir)

    for target in targets:
        # We already built the gypfiles...
        if target == TARGET_GYP:
            continue

        cd(msbuild_working_dir)
        runcommand(
            ('msbuild /nologo /property:Configuration=%s'
            ' /target:%s /verbosity:minimal %s.sln') % (
                BUILDTYPE, target, target))
        runcommand('xcopy /y %s\* %s' % (
            msbuild_output_dir, final_output_dir))


def Make(args):
    """Main function.

    parameters:
        args: command line arguments as a list of strings
    """
    # handle any variable-setting parameters or special targets
    global BUILDTYPE
    targets = []
    for arg in args:
        if arg == TARGET_ALL:
            targets.extend(LIST_OF_ALL_TARGETS)
        elif arg == TARGET_CLEAN:
            MakeClean()
        elif arg.startswith('BUILDTYPE='):
            BUILDTYPE = arg[10:]
        elif arg.startswith('GYP_DEFINES='):
            os.environ['GYP_DEFINES'] = arg[12:]
        else:
            targets.append(arg)

    # if there are no remaining targets, we're done
    if not targets:
        sys.exit(0)

    # dispatch to appropriate Make<Platform>() variant.
    if os.name == 'nt':
        MakeWindows(targets)
        sys.exit(0)
    elif os.name == 'posix':
        if sys.platform == 'darwin':
            print 'Mac developers should not run this script; see ' \
                'http://code.google.com/p/skia/wiki/GettingStartedOnMac'
            sys.exit(1)
        elif sys.platform == 'cygwin':
            print 'Windows development on Cygwin is not currently supported; ' \
                'see http://code.google.com/p/skia/wiki/GettingStartedOnWindows'
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


# main()
Make(sys.argv[1:])

    
