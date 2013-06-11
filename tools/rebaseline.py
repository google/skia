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

# System-level imports
import argparse
import os
import subprocess
import sys
import urllib2

# Imports from within Skia
#
# Make sure that they are in the PYTHONPATH, but add them at the *end*
# so any that are already in the PYTHONPATH will be preferred.
GM_DIRECTORY = os.path.realpath(
    os.path.join(os.path.dirname(os.path.dirname(__file__)), 'gm'))
if GM_DIRECTORY not in sys.path:
    sys.path.append(GM_DIRECTORY)
import gm_json


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


class CommandFailedException(Exception):
    pass

class Rebaseliner(object):

    # params:
    #  json_base_url: base URL from which to read json_filename
    #  json_filename: filename (under json_base_url) from which to read a
    #                 summary of results; typically "actual-results.json"
    #  subdirs: which platform subdirectories to rebaseline; if not specified,
    #           rebaseline all platform subdirectories
    #  tests: list of tests to rebaseline, or None if we should rebaseline
    #         whatever files the JSON results summary file tells us to
    #  configs: which configs to run for each test; this should only be
    #           specified if the list of tests was also specified (otherwise,
    #           the JSON file will give us test names and configs)
    #  dry_run: if True, instead of actually downloading files or adding
    #           files to checkout, display a list of operations that
    #           we would normally perform
    def __init__(self, json_base_url, json_filename,
                 subdirs=None, tests=None, configs=None, dry_run=False):
        if configs and not tests:
            raise ValueError('configs should only be specified if tests ' +
                             'were specified also')
        self._tests = tests
        self._configs = configs
        if not subdirs:
            self._subdirs = sorted(SUBDIR_MAPPING.keys())
            self._missing_json_is_fatal = False
        else:
            self._subdirs = subdirs
            self._missing_json_is_fatal = True
        self._json_base_url = json_base_url
        self._json_filename = json_filename
        self._dry_run = dry_run
        self._is_svn_checkout = (
            os.path.exists('.svn') or
            os.path.exists(os.path.join(os.pardir, '.svn')))
        self._is_git_checkout = (
            os.path.exists('.git') or
            os.path.exists(os.path.join(os.pardir, '.git')))

    # If dry_run is False, execute subprocess.call(cmd).
    # If dry_run is True, print the command we would have otherwise run.
    # Raises a CommandFailedException if the command fails.
    def _Call(self, cmd):
        if self._dry_run:
            print '%s' % ' '.join(cmd)
            return
        if subprocess.call(cmd) != 0:
            raise CommandFailedException('error running command: ' +
                                         ' '.join(cmd))

    # Download a single file, raising a CommandFailedException if it fails.
    def _DownloadFile(self, source_url, dest_filename):
        # Download into a temporary file and then rename it afterwards,
        # so that we don't corrupt the existing file if it fails midway thru.
        temp_filename = os.path.join(os.path.dirname(dest_filename),
                                     '.temp-' + os.path.basename(dest_filename))

        # TODO(epoger): Replace calls to "curl"/"mv" (which will only work on
        # Unix) with a Python HTTP library (which should work cross-platform)
        self._Call([ 'curl', '--fail', '--silent', source_url,
                     '--output', temp_filename ])
        self._Call([ 'mv', temp_filename, dest_filename ])

    # Returns the full contents of a URL, as a single string.
    #
    # Unlike standard URL handling, we allow relative "file:" URLs;
    # for example, "file:one/two" resolves to the file ./one/two
    # (relative to current working dir)
    def _GetContentsOfUrl(self, url):
        file_prefix = 'file:'
        if url.startswith(file_prefix):
            filename = url[len(file_prefix):]
            return open(filename, 'r').read()
        else:
            return urllib2.urlopen(url).read()

    # Returns a list of files that require rebaselining.
    #
    # Note that this returns a list of FILES, like this:
    #  ['imageblur_565.png', 'xfermodes_pdf.png']
    # rather than a list of TESTS, like this:
    #  ['imageblur', 'xfermodes']
    #
    # If the JSON actual result summary file cannot be loaded, the behavior
    # depends on self._missing_json_is_fatal:
    # - if true: execution will halt with an exception
    # - if false: we will log an error message but return an empty list so we
    #   go on to the next platform
    #
    # params:
    #  json_url: URL pointing to a JSON actual result summary file
    #
    # TODO(epoger): add a parameter indicating whether "no-comparison"
    # results (those for which we don't have any expectations yet)
    # should be rebaselined.  For now, we only return failed expectations.
    def _GetFilesToRebaseline(self, json_url):
        if self._dry_run:
            print ''
            print '#'
        print ('# Getting files to rebaseline from JSON summary URL %s ...'
               % json_url)
        try:
            json_contents = self._GetContentsOfUrl(json_url)
        except urllib2.HTTPError:
            message = 'unable to load JSON summary URL %s' % json_url
            if self._missing_json_is_fatal:
                raise ValueError(message)
            else:
                print '# %s' % message
                return []

        json_dict = gm_json.LoadFromString(json_contents)
        actual_results = json_dict[gm_json.JSONKEY_ACTUALRESULTS]

        files_to_rebaseline = []
        failed_results = actual_results[gm_json.JSONKEY_ACTUALRESULTS_FAILED]
        if failed_results:
            files_to_rebaseline.extend(failed_results.keys())

        print '# ... found files_to_rebaseline %s' % files_to_rebaseline
        if self._dry_run:
            print '#'
        return files_to_rebaseline

    # Rebaseline a single file.
    def _RebaselineOneFile(self, expectations_subdir, builder_name,
                           infilename, outfilename):
        if self._dry_run:
            print ''
        print '# ' + infilename
        url = ('http://skia-autogen.googlecode.com/svn/gm-actual/' +
               expectations_subdir + '/' + builder_name + '/' +
               expectations_subdir + '/' + infilename)

        # Try to download this file, but if that fails, keep going...
        #
        # This not treated as a fatal failure because not all
        # platforms generate all configs (e.g., Android does not
        # generate PDF).
        #
        # We could tweak the list of configs within this tool to
        # reflect which combinations the bots actually generate, and
        # then fail if any of those expected combinations are
        # missing... but then this tool would become useless every
        # time someone tweaked the configs on the bots without
        # updating this script.
        try:
            self._DownloadFile(source_url=url, dest_filename=outfilename)
        except CommandFailedException:
            print '# Couldn\'t fetch ' + url
            return

        # Add this file to version control (if it isn't already).
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
        if self._dry_run:
            print ''
        print '# ' + expectations_subdir + ':'
        for config in configs:
            infilename = test + '_' + config + '.png'
            outfilename = os.path.join(expectations_subdir, infilename);
            self._RebaselineOneFile(expectations_subdir=expectations_subdir,
                                    builder_name=builder_name,
                                    infilename=infilename,
                                    outfilename=outfilename)

    # Rebaseline all platforms/tests/types we specified in the constructor.
    def RebaselineAll(self):
        for subdir in self._subdirs:
            if not subdir in SUBDIR_MAPPING.keys():
                raise Exception(('unrecognized platform subdir "%s"; ' +
                                 'should be one of %s') % (
                                     subdir, SUBDIR_MAPPING.keys()))
            builder_name = SUBDIR_MAPPING[subdir]
            if self._tests:
                for test in self._tests:
                    self._RebaselineOneTest(expectations_subdir=subdir,
                                            builder_name=builder_name,
                                            test=test)
            else:  # get the raw list of files that need rebaselining from JSON
                json_url = '/'.join([self._json_base_url,
                                     subdir, builder_name, subdir,
                                     self._json_filename])
                filenames = self._GetFilesToRebaseline(json_url=json_url)
                for filename in filenames:
                    outfilename = os.path.join(subdir, filename);
                    self._RebaselineOneFile(expectations_subdir=subdir,
                                            builder_name=builder_name,
                                            infilename=filename,
                                            outfilename=outfilename)

# main...

parser = argparse.ArgumentParser()
parser.add_argument('--configs', metavar='CONFIG', nargs='+',
                    help='which configurations to rebaseline, e.g. ' +
                    '"--configs 565 8888"; if unspecified, run a default ' +
                    'set of configs. This should ONLY be specified if ' +
                    '--tests has also been specified.')
parser.add_argument('--dry-run', action='store_true',
                    help='instead of actually downloading files or adding ' +
                    'files to checkout, display a list of operations that ' +
                    'we would normally perform')
parser.add_argument('--json-base-url',
                    help='base URL from which to read JSON_FILENAME ' +
                    'files; defaults to %(default)s',
                    default='http://skia-autogen.googlecode.com/svn/gm-actual')
parser.add_argument('--json-filename',
                    help='filename (under JSON_BASE_URL) to read a summary ' +
                    'of results from; defaults to %(default)s',
                    default='actual-results.json')
parser.add_argument('--subdirs', metavar='SUBDIR', nargs='+',
                    help='which platform subdirectories to rebaseline; ' +
                    'if unspecified, rebaseline all subdirs, same as ' +
                    '"--subdirs %s"' % ' '.join(sorted(SUBDIR_MAPPING.keys())))
parser.add_argument('--tests', metavar='TEST', nargs='+',
                    help='which tests to rebaseline, e.g. ' +
                    '"--tests aaclip bigmatrix"; if unspecified, then all ' +
                    'failing tests (according to the actual-results.json ' +
                    'file) will be rebaselined.')
args = parser.parse_args()
rebaseliner = Rebaseliner(tests=args.tests, configs=args.configs,
                          subdirs=args.subdirs, dry_run=args.dry_run,
                          json_base_url=args.json_base_url,
                          json_filename=args.json_filename)
rebaseliner.RebaselineAll()
