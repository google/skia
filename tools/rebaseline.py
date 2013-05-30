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

import argparse
import os
import subprocess
import sys
import tempfile

# Mapping of gm-expectations subdir (under
# https://skia.googlecode.com/svn/gm-expected/ )
# to builder name (see list at http://108.170.217.252:10117/builders )
SUBDIR_MAPPING = {
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


class Rebaseliner(object):

    # params:
    #  tests: list of tests to rebaseline
    #  configs: which configs to run for each test
    #  subdirs: which platform subdirectories to rebaseline; if an empty list,
    #           rebaseline all platform subdirectories
    #  dry_run: if True, instead of actually downloading files or adding
    #           files to checkout, display a list of operations that
    #           we would normally perform
    def __init__(self, tests, configs=[], subdirs=[], dry_run=False):
        if not tests:
            raise Exception('at least one test must be specified')
        self._tests = tests
        self._configs = configs
        if not subdirs:
            self._subdirs = sorted(SUBDIR_MAPPING.keys())
        else:
            self._subdirs = subdirs
        self._dry_run = dry_run
        self._is_svn_checkout = (
            os.path.exists('.svn') or
            os.path.exists(os.path.join(os.pardir, '.svn')))
        self._is_git_checkout = (
            os.path.exists('.git') or
            os.path.exists(os.path.join(os.pardir, '.git')))

    # Execute subprocess.call(), unless dry_run is True
    def _Call(self, cmd, stdout=None):
        if self._dry_run:
            print '%s' % ' '.join(cmd)
            return 0
        if stdout:
            return subprocess.call(cmd, stdout=stdout)
        else:
            return subprocess.call(cmd)

    # Rebaseline a single file.
    def _RebaselineOneFile(self, expectations_subdir, builder_name,
                           infilename, outfilename):
        url = ('http://skia-autogen.googlecode.com/svn/gm-actual/' +
               expectations_subdir + '/' + builder_name + '/' +
               expectations_subdir + '/' + infilename)
        cmd = [ 'curl', '--fail', '--silent', url ]
        temp = tempfile.NamedTemporaryFile()
        ret = self._Call(cmd, stdout=temp)
        if ret != 0:
            print '# Couldn\'t fetch ' + url
            return
        cmd = [ 'cp', temp.name, outfilename ]
        self._Call(cmd);
        if self._is_svn_checkout:
            cmd = [ 'svn', 'add', '--quiet', outfilename ]
            self._Call(cmd)
            cmd = [ 'svn', 'propset', '--quiet', 'svn:mime-type', 'image/png',
                    outfilename ];
            self._Call(cmd)
        elif self._is_git_checkout:
            cmd = [ 'git', 'add', outfilename ]
            self._Call(cmd)

    # Rebaseline the given configs for a single test.
    #
    # params:
    #  expectations_subdir
    #  builder_name
    #  test: a single test to rebaseline
    def _RebaselineOneTest(self, expectations_subdir, builder_name, test):
        if self._configs:
            configs = self._configs
        else:
            if (expectations_subdir == 'base-shuttle-win7-intel-angle'):
                configs = [ 'angle', 'anglemsaa16' ]
            else:
                configs = [ '565', '8888', 'gpu', 'pdf', 'mesa', 'msaa16',
                            'msaa4' ]
        print '# ' + expectations_subdir + ':'
        for config in configs:
            infilename = test + '_' + config + '.png'
            print '# ' + infilename
            outfilename = os.path.join(expectations_subdir, infilename);
            self._RebaselineOneFile(expectations_subdir=expectations_subdir,
                                    builder_name=builder_name,
                                    infilename=infilename,
                                    outfilename=outfilename)

    # Rebaseline all platforms/tests/types we specified in the constructor.
    def RebaselineAll(self):
        for test in self._tests:
            for subdir in self._subdirs:
                if not subdir in SUBDIR_MAPPING.keys():
                    raise Exception(('unrecognized platform subdir "%s"; ' +
                                     'should be one of %s') % (
                                         subdir, SUBDIR_MAPPING.keys()))
                builder_name = SUBDIR_MAPPING[subdir]
                self._RebaselineOneTest(expectations_subdir=subdir,
                                        builder_name=builder_name,
                                        test=test)


# main...

parser = argparse.ArgumentParser()
parser.add_argument('--configs', metavar='CONFIG', nargs='+',
                    help='which configurations to rebaseline, e.g. ' +
                    '"--configs 565 8888"; if unspecified, run a default ' +
                    'set of configs')
parser.add_argument('--dry_run', action='store_true',
                    help='instead of actually downloading files or adding ' +
                    'files to checkout, display a list of operations that ' +
                    'we would normally perform')
parser.add_argument('--subdirs', metavar='SUBDIR', nargs='+',
                    help='which platform subdirectories to rebaseline; ' +
                    'if unspecified, rebaseline all subdirs, same as ' +
                    '"--subdirs %s"' % ' '.join(sorted(SUBDIR_MAPPING.keys())))
parser.add_argument('--tests', metavar='TEST', nargs='+', required=True,
                    help='which tests to rebaseline, e.g. ' +
                    '"--tests aaclip bigmatrix"')
args = parser.parse_args()
rebaseliner = Rebaseliner(tests=args.tests, configs=args.configs,
                          subdirs=args.subdirs, dry_run=args.dry_run)
rebaseliner.RebaselineAll()
