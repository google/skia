#!/usr/bin/python

'''
Copyright 2012 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

'''
Rebaselines the given GM tests, on all bots and all configurations.
Must be run from the gm-expected directory.  If run from a git or SVN
checkout, the files will be added to the staging area for commit.
'''

import os
import subprocess
import sys
import tempfile

# Mapping of gm-expectations subdir (under
# https://skia.googlecode.com/svn/gm-expected/ )
# to builder name (see list at http://108.170.217.252:10117/builders )
subdir_mapping = {
   'base-shuttle-win7-intel-float':
    'Test-Win7-ShuttleA-HD2000-x86-Release',
   'base-shuttle-win7-intel-angle':
    'Test-Win7-ShuttleA-HD2000-x86-Release-ANGLE',
   'base-shuttle-win7-intel-directwrite':
    'Test-Win7-ShuttleA-HD2000-x86-Release-DirectWrite',
   'base-shuttle_ubuntu12_ati5770':
    'Test-Ubuntu12-ShuttleA-ATI5770-x86_64-Release',
   'base-macmini':
    'Test-Mac10.6-MacMini4.1-GeForce320M-x86-Release',
   'base-macmini-lion-float':
    'Test-Mac10.7-MacMini4.1-GeForce320M-x86-Release',
   'base-android-galaxy-nexus':
    'Test-Android-GalaxyNexus-SGX540-Arm7-Debug',
   'base-android-nexus-7':
    'Test-Android-Nexus7-Tegra3-Arm7-Release',
   'base-android-nexus-s':
    'Test-Android-NexusS-SGX540-Arm7-Release',
   'base-android-xoom':
    'Test-Android-Xoom-Tegra2-Arm7-Release',
   'base-android-nexus-10':
    'Test-Android-Nexus10-MaliT604-Arm7-Release',
}

IS_SVN_CHECKOUT = (os.path.exists('.svn') or
                   os.path.exists(os.path.join('..', '.svn')))
IS_GIT_CHECKOUT = (os.path.exists('.git') or
                   os.path.exists(os.path.join('..', '.git')))


# Rebaseline a single file.
def RebaselineOneFile(expectations_subdir, builder_name,
                      infilename, outfilename):
    url = ('http://skia-autogen.googlecode.com/svn/gm-actual/' +
           expectations_subdir + '/' + builder_name + '/' +
           expectations_subdir + '/' + infilename)
    cmd = [ 'curl', '--fail', '--silent', url ]
    temp = tempfile.NamedTemporaryFile()
    ret = subprocess.call(cmd, stdout=temp)
    if ret != 0:
        print 'Couldn\'t fetch ' + url
        return
    cmd = [ 'cp', temp.name, outfilename ]
    subprocess.call(cmd);
    if IS_SVN_CHECKOUT:
        cmd = [ 'svn', 'add', '--quiet', outfilename ]
        subprocess.call(cmd)
        cmd = [ 'svn', 'propset', '--quiet', 'svn:mime-type', 'image/png',
                outfilename ];
        subprocess.call(cmd)
    elif IS_GIT_CHECKOUT:
        cmd = [ 'git', 'add', outfilename ]
        subprocess.call(cmd)


# Rebaseline all testtypes for a single test.
def RebaselineOneTest(expectations_subdir, builder_name, testname):
    if (expectations_subdir == 'base-shuttle-win7-intel-angle'):
        testtypes = [ 'angle', 'anglemsaa16' ]
    else:
        testtypes = [ '565', '8888', 'gpu', 'pdf', 'mesa', 'msaa16', 'msaa4' ]
    print expectations_subdir + ':'
    for testtype in testtypes:
        infilename = testname + '_' + testtype + '.png'
        print infilename
        outfilename = os.path.join(expectations_subdir, infilename);
        RebaselineOneFile(expectations_subdir=expectations_subdir,
                          builder_name=builder_name,
                          infilename=infilename,
                          outfilename=outfilename)



if len(sys.argv) < 2:
    print ('Usage:  ' + os.path.basename(sys.argv[0]) +
           ' <testname> [ <testname> ... ]')
    exit(1)

for testname in sys.argv[1:]:
    for expectations_subdir in sorted(subdir_mapping.keys()):
        builder_name = subdir_mapping[expectations_subdir]
        RebaselineOneTest(expectations_subdir=expectations_subdir,
                          builder_name=builder_name,
                          testname=testname)
