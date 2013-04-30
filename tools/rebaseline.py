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

import os, subprocess, sys, tempfile

pairs = [ 
   ['base-shuttle-win7-intel-float',
    'Test-Win7-ShuttleA-HD2000-x86-Release'],
   ['base-shuttle-win7-intel-angle',
    'Test-Win7-ShuttleA-HD2000-x86-Release-ANGLE'],
   ['base-shuttle-win7-intel-directwrite',
    'Test-Win7-ShuttleA-HD2000-x86-Release-DirectWrite'],
   ['base-shuttle_ubuntu12_ati5770',
    'Test-Ubuntu12-ShuttleA-ATI5770-x86_64-Release'],
   ['base-macmini',
    'Test-Mac10.6-MacMini4.1-GeForce320M-x86-Release'],
   ['base-macmini-lion-float',
    'Test-Mac10.7-MacMini4.1-GeForce320M-x86-Release'],
   ['base-android-galaxy-nexus',
    'Test-Android-GalaxyNexus-SGX540-Arm7-Release'],
   ['base-android-nexus-7',
    'Test-Android-Nexus7-Tegra3-Arm7-Release'],
   ['base-android-nexus-s',
    'Test-Android-NexusS-SGX540-Arm7-Release'],
   ['base-android-xoom',
    'Test-Android-Xoom-Tegra2-Arm7-Release'],
]

if len(sys.argv) < 2:
    print 'Usage:  ' + os.path.basename(sys.argv[0]) + ' <testname> '
    '[ <testname> ... ]'
    exit(1)

is_svn_checkout = os.path.exists('.svn') or os.path.exists(os.path.join('..', '.svn') )
is_git_checkout = os.path.exists('.git') or os.path.exists(os.path.join('..', '.git'))

for testname in sys.argv[1:]:
    for pair in pairs:
        if (pair[0] == 'base-shuttle-win7-intel-angle'):
            testtypes = [ 'angle' ]
        else:
            testtypes = [ '565', '8888', 'gpu', 'pdf', 'mesa' ]
        print pair[0] + ':'
        for testtype in testtypes:
            infilename = testname + '_' + testtype + '.png'
            print infilename

            url = 'http://skia-autogen.googlecode.com/svn/gm-actual/' + pair[0] + '/' + pair[1] + '/' + pair[0] + '/' + infilename
            cmd = [ 'curl', '--fail', '--silent', url ]
            temp = tempfile.NamedTemporaryFile()
            ret = subprocess.call(cmd, stdout=temp)
            if ret != 0:
                print 'Couldn\'t fetch ' + url
                continue
            outfilename = os.path.join(pair[0], infilename);
            cmd = [ 'cp', temp.name, outfilename ]
            subprocess.call(cmd);
            if is_svn_checkout:
                cmd = [ 'svn', 'add', '--quiet', outfilename ]
                subprocess.call(cmd)
                cmd = [ 'svn', 'propset', '--quiet', 'svn:mime-type', 'image/png', outfilename ];
                subprocess.call(cmd)
            elif is_git_checkout:
                cmd = [ 'git', 'add', outfilename ]
                subprocess.call(cmd)
