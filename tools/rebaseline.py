#!/usr/bin/python

'''
Copyright 2012 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

'''
Rebaselines a single GM test, on all bots and all configurations.
Must be run from an SVN checkout of the gm-expected directory.
'''

import os, subprocess, sys, tempfile

pairs = [ 
   ['base-shuttle-win7-intel-float',
    'Skia_Shuttle_Win7_Intel_Float_Release_32'],
#    ['base-shuttle-win7-intel-angle',
#     'Skia_Shuttle_Win7_Intel_Float_ANGLE_Release_32'],
#    ['base-shuttle-win7-intel-directwrite',
#     'Skia_Shuttle_Win7_Intel_Float_DirectWrite_Release_32'],
   ['base-shuttle_ubuntu12_ati5770',
    'Skia_Shuttle_Ubuntu12_ATI5770_Float_Release_64'],
   ['base-macmini',
    'Skia_Mac_Float_Release_32'],
   ['base-macmini-lion-float',
    'Skia_MacMiniLion_Float_Release_32'],
   ['base-android-galaxy-nexus',
    'Skia_GalaxyNexus_4-1_Float_Release_32'],
   ['base-android-nexus-7',
    'Skia_Nexus7_4-1_Float_Release_32'],
   ['base-android-nexus-s',
    'Skia_NexusS_4-1_Float_Release_32'],
   ['base-android-xoom',
    'Skia_Xoom_4-1_Float_Release_32'],
]

if len(sys.argv) != 2:
    print 'Usage:  ' + os.path.basename(sys.argv[0]) + ' <testname>'
    exit(1)

testname = sys.argv[1]
testtypes = [ '4444', '565', '8888', 'gpu', 'pdf' ]

for pair in pairs:
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
        cmd = [ 'svn', 'add', '--quiet', outfilename ]
        subprocess.call(cmd)
