#!/usr/bin/python

'''
Copyright 2012 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

'''
Rebaselines the given GM tests, on all bots and all configurations.
'''

# System-level imports
import argparse
import json
import os
import re
import subprocess
import sys
import urllib2

# Imports from within Skia
#
# We need to add the 'gm' directory, so that we can import gm_json.py within
# that directory.  That script allows us to parse the actual-results.json file
# written out by the GM tool.
# Make sure that the 'gm' dir is in the PYTHONPATH, but add it at the *end*
# so any dirs that are already in the PYTHONPATH will be preferred.
#
# This assumes that the 'gm' directory has been checked out as a sibling of
# the 'tools' directory containing this script, which will be the case if
# 'trunk' was checked out as a single unit.
GM_DIRECTORY = os.path.realpath(
    os.path.join(os.path.dirname(os.path.dirname(__file__)), 'gm'))
if GM_DIRECTORY not in sys.path:
  sys.path.append(GM_DIRECTORY)
import gm_json

# TODO(epoger): In the long run, we want to build this list automatically,
# but for now we hard-code it until we can properly address
# https://code.google.com/p/skia/issues/detail?id=1544
# ('live query of builder list makes rebaseline.py slow to start up')
TEST_BUILDERS = [
    'Test-Android-GalaxyNexus-SGX540-Arm7-Debug',
    'Test-Android-GalaxyNexus-SGX540-Arm7-Release',
    'Test-Android-IntelRhb-SGX544-x86-Debug',
    'Test-Android-IntelRhb-SGX544-x86-Release',
    'Test-Android-Nexus10-MaliT604-Arm7-Debug',
    'Test-Android-Nexus10-MaliT604-Arm7-Release',
    'Test-Android-Nexus4-Adreno320-Arm7-Debug',
    'Test-Android-Nexus4-Adreno320-Arm7-Release',
    'Test-Android-Nexus7-Tegra3-Arm7-Debug',
    'Test-Android-Nexus7-Tegra3-Arm7-Release',
    'Test-Android-NexusS-SGX540-Arm7-Debug',
    'Test-Android-NexusS-SGX540-Arm7-Release',
    'Test-Android-Xoom-Tegra2-Arm7-Debug',
    'Test-Android-Xoom-Tegra2-Arm7-Release',
    'Test-ChromeOS-Alex-GMA3150-x86-Debug',
    'Test-ChromeOS-Alex-GMA3150-x86-Release',
    'Test-ChromeOS-Daisy-MaliT604-Arm7-Debug',
    'Test-ChromeOS-Daisy-MaliT604-Arm7-Release',
    'Test-ChromeOS-Link-HD4000-x86_64-Debug',
    'Test-ChromeOS-Link-HD4000-x86_64-Release',
    'Test-Mac10.6-MacMini4.1-GeForce320M-x86-Debug',
    'Test-Mac10.6-MacMini4.1-GeForce320M-x86-Release',
    'Test-Mac10.6-MacMini4.1-GeForce320M-x86_64-Debug',
    'Test-Mac10.6-MacMini4.1-GeForce320M-x86_64-Release',
    'Test-Mac10.7-MacMini4.1-GeForce320M-x86-Debug',
    'Test-Mac10.7-MacMini4.1-GeForce320M-x86-Release',
    'Test-Mac10.7-MacMini4.1-GeForce320M-x86_64-Debug',
    'Test-Mac10.7-MacMini4.1-GeForce320M-x86_64-Release',
    'Test-Mac10.8-MacMini4.1-GeForce320M-x86-Debug',
    'Test-Mac10.8-MacMini4.1-GeForce320M-x86-Release',
    'Test-Mac10.8-MacMini4.1-GeForce320M-x86_64-Debug',
    'Test-Mac10.8-MacMini4.1-GeForce320M-x86_64-Release',
    'Test-Ubuntu12-ShuttleA-ATI5770-x86-Debug',
    'Test-Ubuntu12-ShuttleA-ATI5770-x86-Release',
    'Test-Ubuntu12-ShuttleA-ATI5770-x86_64-Debug',
    'Test-Ubuntu12-ShuttleA-ATI5770-x86_64-Release',
    'Test-Ubuntu12-ShuttleA-HD2000-x86_64-Release-Valgrind',
    'Test-Ubuntu12-ShuttleA-NoGPU-x86_64-Debug',
    'Test-Ubuntu13-ShuttleA-HD2000-x86_64-Debug-ASAN',
    'Test-Win7-ShuttleA-HD2000-x86-Debug',
    'Test-Win7-ShuttleA-HD2000-x86-Debug-ANGLE',
    'Test-Win7-ShuttleA-HD2000-x86-Debug-DirectWrite',
    'Test-Win7-ShuttleA-HD2000-x86-Release',
    'Test-Win7-ShuttleA-HD2000-x86-Release-ANGLE',
    'Test-Win7-ShuttleA-HD2000-x86-Release-DirectWrite',
    'Test-Win7-ShuttleA-HD2000-x86_64-Debug',
    'Test-Win7-ShuttleA-HD2000-x86_64-Release',
]

# TODO: Get this from builder_name_schema in buildbot.
TRYBOT_SUFFIX = '-Trybot'


class _InternalException(Exception):
  pass

class ExceptionHandler(object):
  """ Object that handles exceptions, either raising them immediately or
  collecting them to display later on."""

  # params:
  def __init__(self, keep_going_on_failure=False):
    """
    params:
      keep_going_on_failure: if False, report failures and quit right away;
                             if True, collect failures until
                             ReportAllFailures() is called
    """
    self._keep_going_on_failure = keep_going_on_failure
    self._failures_encountered = []

  def RaiseExceptionOrContinue(self):
    """ We have encountered an exception; either collect the info and keep
    going, or exit the program right away."""
    # Get traceback information about the most recently raised exception.
    exc_info = sys.exc_info()

    if self._keep_going_on_failure:
      print >> sys.stderr, ('WARNING: swallowing exception %s' %
                            repr(exc_info[1]))
      self._failures_encountered.append(exc_info)
    else:
      print >> sys.stderr, (
          '\nHalting at first exception.\n' +
          'Please file a bug to epoger@google.com at ' +
          'https://code.google.com/p/skia/issues/entry, containing the ' +
          'command you ran and the following stack trace.\n\n' +
          'Afterwards, you can re-run with the --keep-going-on-failure ' +
          'option set.\n')
      raise exc_info[1], None, exc_info[2]

  def ReportAllFailures(self):
    if self._failures_encountered:
      print >> sys.stderr, ('Encountered %d failures (see above).' %
                            len(self._failures_encountered))
      sys.exit(1)


# Object that rebaselines a JSON expectations file (not individual image files).
class JsonRebaseliner(object):

  # params:
  #  expectations_root: root directory of all expectations JSON files
  #  expectations_input_filename: filename (under expectations_root) of JSON
  #                               expectations file to read; typically
  #                               "expected-results.json"
  #  expectations_output_filename: filename (under expectations_root) to
  #                                which updated expectations should be
  #                                written; typically the same as
  #                                expectations_input_filename, to overwrite
  #                                the old content
  #  actuals_base_url: base URL from which to read actual-result JSON files
  #  actuals_filename: filename (under actuals_base_url) from which to read a
  #                    summary of results; typically "actual-results.json"
  #  exception_handler: reference to rebaseline.ExceptionHandler object
  #  tests: list of tests to rebaseline, or None if we should rebaseline
  #         whatever files the JSON results summary file tells us to
  #  configs: which configs to run for each test, or None if we should
  #           rebaseline whatever configs the JSON results summary file tells
  #           us to
  #  add_new: if True, add expectations for tests which don't have any yet
  #  add_ignored: if True, add expectations for tests for which failures are
  #               currently ignored
  #  bugs: optional list of bug numbers which pertain to these expectations
  #  notes: free-form text notes to add to all updated expectations
  #  mark_unreviewed: if True, mark these expectations as NOT having been
  #                   reviewed by a human; otherwise, leave that field blank.
  #                   Currently, there is no way to make this script mark
  #                   expectations as reviewed-by-human=True.
  #                   TODO(epoger): Add that capability to a review tool.
  #  mark_ignore_failure: if True, mark failures of a given test as being
  #                       ignored.
  #  from_trybot: if True, read actual-result JSON files generated from a
  #               trybot run rather than a waterfall run.
  def __init__(self, expectations_root, expectations_input_filename,
               expectations_output_filename, actuals_base_url,
               actuals_filename, exception_handler,
               tests=None, configs=None, add_new=False, add_ignored=False,
               bugs=None, notes=None, mark_unreviewed=None,
               mark_ignore_failure=False, from_trybot=False):
    self._expectations_root = expectations_root
    self._expectations_input_filename = expectations_input_filename
    self._expectations_output_filename = expectations_output_filename
    self._tests = tests
    self._configs = configs
    self._actuals_base_url = actuals_base_url
    self._actuals_filename = actuals_filename
    self._exception_handler = exception_handler
    self._add_new = add_new
    self._add_ignored = add_ignored
    self._bugs = bugs
    self._notes = notes
    self._mark_unreviewed = mark_unreviewed
    self._mark_ignore_failure = mark_ignore_failure;
    if self._tests or self._configs:
      self._image_filename_re = re.compile(gm_json.IMAGE_FILENAME_PATTERN)
    else:
      self._image_filename_re = None
    self._using_svn = os.path.isdir(os.path.join(expectations_root, '.svn'))
    self._from_trybot = from_trybot

  # Executes subprocess.call(cmd).
  # Raises an Exception if the command fails.
  def _Call(self, cmd):
    if subprocess.call(cmd) != 0:
      raise _InternalException('error running command: ' + ' '.join(cmd))

  # Returns the full contents of filepath, as a single string.
  # If filepath looks like a URL, try to read it that way instead of as
  # a path on local storage.
  #
  # Raises _InternalException if there is a problem.
  def _GetFileContents(self, filepath):
    if filepath.startswith('http:') or filepath.startswith('https:'):
      try:
        return urllib2.urlopen(filepath).read()
      except urllib2.HTTPError as e:
        raise _InternalException('unable to read URL %s: %s' % (
            filepath, e))
    else:
      return open(filepath, 'r').read()

  # Returns a dictionary of actual results from actual-results.json file.
  #
  # The dictionary returned has this format:
  # {
  #  u'imageblur_565.png': [u'bitmap-64bitMD5', 3359963596899141322],
  #  u'imageblur_8888.png': [u'bitmap-64bitMD5', 4217923806027861152],
  #  u'shadertext3_8888.png': [u'bitmap-64bitMD5', 3713708307125704716]
  # }
  #
  # If the JSON actual result summary file cannot be loaded, logs a warning
  # message and returns None.
  # If the JSON actual result summary file can be loaded, but we have
  # trouble parsing it, raises an Exception.
  #
  # params:
  #  json_url: URL pointing to a JSON actual result summary file
  #  sections: a list of section names to include in the results, e.g.
  #            [gm_json.JSONKEY_ACTUALRESULTS_FAILED,
  #             gm_json.JSONKEY_ACTUALRESULTS_NOCOMPARISON] ;
  #            if None, then include ALL sections.
  def _GetActualResults(self, json_url, sections=None):
    try:
      json_contents = self._GetFileContents(json_url)
    except _InternalException:
      print >> sys.stderr, (
          'could not read json_url %s ; skipping this platform.' %
          json_url)
      return None
    json_dict = gm_json.LoadFromString(json_contents)
    results_to_return = {}
    actual_results = json_dict[gm_json.JSONKEY_ACTUALRESULTS]
    if not sections:
      sections = actual_results.keys()
    for section in sections:
      section_results = actual_results[section]
      if section_results:
        results_to_return.update(section_results)
    return results_to_return

  # Rebaseline all tests/types we specified in the constructor,
  # within this builder's subdirectory in expectations/gm .
  #
  # params:
  #  builder : e.g. 'Test-Win7-ShuttleA-HD2000-x86-Release'
  def RebaselineSubdir(self, builder):
    # Read in the actual result summary, and extract all the tests whose
    # results we need to update.
    results_builder = str(builder)
    if self._from_trybot:
      results_builder = results_builder + TRYBOT_SUFFIX
    actuals_url = '/'.join([self._actuals_base_url, results_builder,
                            self._actuals_filename])
    # Only update results for tests that are currently failing.
    # We don't want to rewrite results for tests that are already succeeding,
    # because we don't want to add annotation fields (such as
    # JSONKEY_EXPECTEDRESULTS_BUGS) except for tests whose expectations we
    # are actually modifying.
    sections = [gm_json.JSONKEY_ACTUALRESULTS_FAILED]
    if self._add_new:
      sections.append(gm_json.JSONKEY_ACTUALRESULTS_NOCOMPARISON)
    if self._add_ignored:
      sections.append(gm_json.JSONKEY_ACTUALRESULTS_FAILUREIGNORED)
    results_to_update = self._GetActualResults(json_url=actuals_url,
                                               sections=sections)

    # Read in current expectations.
    expectations_input_filepath = os.path.join(
        self._expectations_root, builder, self._expectations_input_filename)
    expectations_dict = gm_json.LoadFromFile(expectations_input_filepath)
    expected_results = expectations_dict.get(gm_json.JSONKEY_EXPECTEDRESULTS)
    if not expected_results:
      expected_results = {}
      expectations_dict[gm_json.JSONKEY_EXPECTEDRESULTS] = expected_results

    # Update the expectations in memory, skipping any tests/configs that
    # the caller asked to exclude.
    skipped_images = []
    if results_to_update:
      for (image_name, image_results) in results_to_update.iteritems():
        if self._image_filename_re:
          (test, config) = self._image_filename_re.match(image_name).groups()
          if self._tests:
            if test not in self._tests:
              skipped_images.append(image_name)
              continue
          if self._configs:
            if config not in self._configs:
              skipped_images.append(image_name)
              continue
        if not expected_results.get(image_name):
          expected_results[image_name] = {}
        expected_results[image_name]\
                        [gm_json.JSONKEY_EXPECTEDRESULTS_ALLOWEDDIGESTS]\
                        = [image_results]
        if self._mark_unreviewed:
          expected_results[image_name]\
                          [gm_json.JSONKEY_EXPECTEDRESULTS_REVIEWED]\
                          = False
        if self._mark_ignore_failure:
          expected_results[image_name]\
                          [gm_json.JSONKEY_EXPECTEDRESULTS_IGNOREFAILURE]\
                          = True
        if self._bugs:
          expected_results[image_name]\
                          [gm_json.JSONKEY_EXPECTEDRESULTS_BUGS]\
                          = self._bugs
        if self._notes:
          expected_results[image_name]\
                          [gm_json.JSONKEY_EXPECTEDRESULTS_NOTES]\
                          = self._notes

    # Write out updated expectations.
    expectations_output_filepath = os.path.join(
        self._expectations_root, builder, self._expectations_output_filename)
    gm_json.WriteToFile(expectations_dict, expectations_output_filepath)

    # Mark the JSON file as plaintext, so text-style diffs can be applied.
    # Fixes https://code.google.com/p/skia/issues/detail?id=1442
    if self._using_svn:
      self._Call(['svn', 'propset', '--quiet', 'svn:mime-type',
                  'text/x-json', expectations_output_filepath])

# main...

parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    epilog='Here is the full set of builders we know about:' +
           '\n    '.join([''] + sorted(TEST_BUILDERS)))
parser.add_argument('--actuals-base-url',
                    help=('base URL from which to read files containing JSON '
                          'summaries of actual GM results; defaults to '
                          '%(default)s. To get a specific revision (useful for '
                          'trybots) replace "svn" with "svn-history/r123". '
                          'If SKIMAGE is True, defaults to ' +
                          gm_json.SKIMAGE_ACTUALS_BASE_URL),
                    default='http://skia-autogen.googlecode.com/svn/gm-actual')
parser.add_argument('--actuals-filename',
                    help=('filename (within builder-specific subdirectories '
                          'of ACTUALS_BASE_URL) to read a summary of results '
                          'from; defaults to %(default)s'),
                    default='actual-results.json')
parser.add_argument('--add-new', action='store_true',
                    help=('in addition to the standard behavior of '
                          'updating expectations for failing tests, add '
                          'expectations for tests which don\'t have '
                          'expectations yet.'))
parser.add_argument('--add-ignored', action='store_true',
                    help=('in addition to the standard behavior of '
                          'updating expectations for failing tests, add '
                          'expectations for tests for which failures are '
                          'currently ignored.'))
parser.add_argument('--bugs', metavar='BUG', type=int, nargs='+',
                    help=('Skia bug numbers (under '
                          'https://code.google.com/p/skia/issues/list ) which '
                          'pertain to this set of rebaselines.'))
parser.add_argument('--builders', metavar='BUILDER', nargs='+',
                    help=('which platforms to rebaseline; '
                          'if unspecified, rebaseline all known platforms '
                          '(see below for a list)'))
# TODO(epoger): Add test that exercises --configs argument.
parser.add_argument('--configs', metavar='CONFIG', nargs='+',
                    help=('which configurations to rebaseline, e.g. '
                          '"--configs 565 8888", as a filter over the full set '
                          'of results in ACTUALS_FILENAME; if unspecified, '
                          'rebaseline *all* configs that are available.'))
parser.add_argument('--expectations-filename',
                    help=('filename (under EXPECTATIONS_ROOT) to read '
                          'current expectations from, and to write new '
                          'expectations into (unless a separate '
                          'EXPECTATIONS_FILENAME_OUTPUT has been specified); '
                          'defaults to %(default)s'),
                    default='expected-results.json')
parser.add_argument('--expectations-filename-output',
                    help=('filename (under EXPECTATIONS_ROOT) to write '
                          'updated expectations into; by default, overwrites '
                          'the input file (EXPECTATIONS_FILENAME)'),
                    default='')
parser.add_argument('--expectations-root',
                    help=('root of expectations directory to update-- should '
                          'contain one or more builder subdirectories. '
                          'Defaults to %(default)s. If SKIMAGE is set, '
                          ' defaults to ' + gm_json.SKIMAGE_EXPECTATIONS_ROOT),
                    default=os.path.join('expectations', 'gm'))
parser.add_argument('--keep-going-on-failure', action='store_true',
                    help=('instead of halting at the first error encountered, '
                          'keep going and rebaseline as many tests as '
                          'possible, and then report the full set of errors '
                          'at the end'))
parser.add_argument('--notes',
                    help=('free-form text notes to add to all updated '
                          'expectations'))
# TODO(epoger): Add test that exercises --tests argument.
parser.add_argument('--tests', metavar='TEST', nargs='+',
                    help=('which tests to rebaseline, e.g. '
                          '"--tests aaclip bigmatrix", as a filter over the '
                          'full set of results in ACTUALS_FILENAME; if '
                          'unspecified, rebaseline *all* tests that are '
                          'available.'))
parser.add_argument('--unreviewed', action='store_true',
                    help=('mark all expectations modified by this run as '
                          '"%s": False' %
                          gm_json.JSONKEY_EXPECTEDRESULTS_REVIEWED))
parser.add_argument('--ignore-failure', action='store_true',
                    help=('mark all expectations modified by this run as '
                          '"%s": True' %
                          gm_json.JSONKEY_ACTUALRESULTS_FAILUREIGNORED))
parser.add_argument('--from-trybot', action='store_true',
                    help=('pull the actual-results.json file from the '
                          'corresponding trybot, rather than the main builder'))
parser.add_argument('--skimage', action='store_true',
                    help=('Rebaseline skimage results instead of gm. Defaults '
                          'to False. If True, TESTS and CONFIGS are ignored, '
                          'and ACTUALS_BASE_URL and EXPECTATIONS_ROOT are set '
                          'to alternate defaults, specific to skimage.'))
args = parser.parse_args()
exception_handler = ExceptionHandler(
    keep_going_on_failure=args.keep_going_on_failure)
if args.builders:
  builders = args.builders
  missing_json_is_fatal = True
else:
  builders = sorted(TEST_BUILDERS)
  missing_json_is_fatal = False
if args.skimage:
  # Use a different default if --skimage is specified.
  if args.actuals_base_url == parser.get_default('actuals_base_url'):
    args.actuals_base_url = gm_json.SKIMAGE_ACTUALS_BASE_URL
  if args.expectations_root == parser.get_default('expectations_root'):
    args.expectations_root = gm_json.SKIMAGE_EXPECTATIONS_ROOT
for builder in builders:
  if not builder in TEST_BUILDERS:
    raise Exception(('unrecognized builder "%s"; ' +
                     'should be one of %s') % (
                         builder, TEST_BUILDERS))

  expectations_json_file = os.path.join(args.expectations_root, builder,
                                        args.expectations_filename)
  if os.path.isfile(expectations_json_file):
    rebaseliner = JsonRebaseliner(
        expectations_root=args.expectations_root,
        expectations_input_filename=args.expectations_filename,
        expectations_output_filename=(args.expectations_filename_output or
                                      args.expectations_filename),
        tests=args.tests, configs=args.configs,
        actuals_base_url=args.actuals_base_url,
        actuals_filename=args.actuals_filename,
        exception_handler=exception_handler,
        add_new=args.add_new, add_ignored=args.add_ignored,
        bugs=args.bugs, notes=args.notes,
        mark_unreviewed=args.unreviewed,
        mark_ignore_failure=args.ignore_failure,
        from_trybot=args.from_trybot)
    try:
      rebaseliner.RebaselineSubdir(builder=builder)
    except:
      exception_handler.RaiseExceptionOrContinue()
  else:
    try:
      raise _InternalException('expectations_json_file %s not found' %
                               expectations_json_file)
    except:
      exception_handler.RaiseExceptionOrContinue()

exception_handler.ReportAllFailures()
