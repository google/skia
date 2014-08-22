#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Repackage expected/actual GM results as needed by our HTML rebaseline viewer.
"""

# System-level imports
import argparse
import fnmatch
import logging
import os
import time

# Must fix up PYTHONPATH before importing from within Skia
import rs_fixpypath  # pylint: disable=W0611

# Imports from within Skia
from py.utils import url_utils
import column
import gm_json
import imagediffdb
import imagepair
import imagepairset
import results

EXPECTATION_FIELDS_PASSED_THRU_VERBATIM = [
    results.KEY__EXPECTATIONS__BUGS,
    results.KEY__EXPECTATIONS__IGNOREFAILURE,
    results.KEY__EXPECTATIONS__REVIEWED,
]
FREEFORM_COLUMN_IDS = [
    results.KEY__EXTRACOLUMNS__BUILDER,
    results.KEY__EXTRACOLUMNS__TEST,
]
ORDERED_COLUMN_IDS = [
    results.KEY__EXTRACOLUMNS__RESULT_TYPE,
    results.KEY__EXTRACOLUMNS__BUILDER,
    results.KEY__EXTRACOLUMNS__TEST,
    results.KEY__EXTRACOLUMNS__CONFIG,
]

TRUNK_DIRECTORY = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
DEFAULT_EXPECTATIONS_DIR = os.path.join(TRUNK_DIRECTORY, 'expectations', 'gm')
DEFAULT_IGNORE_FAILURES_FILE = 'ignored-tests.txt'

IMAGEPAIR_SET_DESCRIPTIONS = ('expected image', 'actual image')


class ExpectationComparisons(results.BaseComparisons):
  """Loads actual and expected GM results into an ImagePairSet.

  Loads actual and expected results from all builders, except for those skipped
  by _ignore_builder().

  Once this object has been constructed, the results (in self._results[])
  are immutable.  If you want to update the results based on updated JSON
  file contents, you will need to create a new ExpectationComparisons object."""

  def __init__(self, image_diff_db, actuals_root=results.DEFAULT_ACTUALS_DIR,
               expected_root=DEFAULT_EXPECTATIONS_DIR,
               ignore_failures_file=DEFAULT_IGNORE_FAILURES_FILE,
               diff_base_url=None, builder_regex_list=None):
    """
    Args:
      image_diff_db: instance of ImageDiffDB we use to cache the image diffs
      actuals_root: root directory containing all actual-results.json files
      expected_root: root directory containing all expected-results.json files
      ignore_failures_file: if a file with this name is found within
          expected_root, ignore failures for any tests listed in the file
      diff_base_url: base URL within which the client should look for diff
          images; if not specified, defaults to a "file:///" URL representation
          of image_diff_db's storage_root
      builder_regex_list: List of regular expressions specifying which builders
          we will process. If None, process all builders.
    """
    super(ExpectationComparisons, self).__init__()
    time_start = int(time.time())
    if builder_regex_list != None:
      self.set_match_builders_pattern_list(builder_regex_list)
    self._image_diff_db = image_diff_db
    self._diff_base_url = (
        diff_base_url or
        url_utils.create_filepath_url(image_diff_db.storage_root))
    self._actuals_root = actuals_root
    self._expected_root = expected_root
    self._ignore_failures_on_these_tests = []
    if ignore_failures_file:
      self._ignore_failures_on_these_tests = (
          ExpectationComparisons._read_noncomment_lines(
              os.path.join(expected_root, ignore_failures_file)))
    self._load_actual_and_expected()
    self._timestamp = int(time.time())
    logging.info('Results complete; took %d seconds.' %
                 (self._timestamp - time_start))

  def edit_expectations(self, modifications):
    """Edit the expectations stored within this object and write them back
    to disk.

    Note that this will NOT update the results stored in self._results[] ;
    in order to see those updates, you must instantiate a new
    ExpectationComparisons object based on the (now updated) files on disk.

    Args:
      modifications: a list of dictionaries, one for each expectation to update:

         [
           {
             imagepair.KEY__IMAGEPAIRS__EXPECTATIONS: {
               results.KEY__EXPECTATIONS__BUGS: [123, 456],
               results.KEY__EXPECTATIONS__IGNOREFAILURE: false,
               results.KEY__EXPECTATIONS__REVIEWED: true,
             },
             imagepair.KEY__IMAGEPAIRS__EXTRACOLUMNS: {
               results.KEY__EXTRACOLUMNS__BUILDER: 'Test-Mac10.6-MacMini4.1-GeForce320M-x86-Debug',
               results.KEY__EXTRACOLUMNS__CONFIG: '8888',
               results.KEY__EXTRACOLUMNS__TEST: 'bigmatrix',
             },
             results.KEY__IMAGEPAIRS__IMAGE_B_URL: 'bitmap-64bitMD5/bigmatrix/10894408024079689926.png',
           },
           ...
         ]

    """
    expected_builder_dicts = self._read_builder_dicts_from_root(
        self._expected_root)
    for mod in modifications:
      image_name = results.IMAGE_FILENAME_FORMATTER % (
          mod[imagepair.KEY__IMAGEPAIRS__EXTRACOLUMNS]
             [results.KEY__EXTRACOLUMNS__TEST],
          mod[imagepair.KEY__IMAGEPAIRS__EXTRACOLUMNS]
             [results.KEY__EXTRACOLUMNS__CONFIG])
      _, hash_type, hash_digest = gm_json.SplitGmRelativeUrl(
          mod[imagepair.KEY__IMAGEPAIRS__IMAGE_B_URL])
      allowed_digests = [[hash_type, int(hash_digest)]]
      new_expectations = {
          gm_json.JSONKEY_EXPECTEDRESULTS_ALLOWEDDIGESTS: allowed_digests,
      }
      for field in EXPECTATION_FIELDS_PASSED_THRU_VERBATIM:
        value = mod[imagepair.KEY__IMAGEPAIRS__EXPECTATIONS].get(field)
        if value is not None:
          new_expectations[field] = value
      builder_dict = expected_builder_dicts[
          mod[imagepair.KEY__IMAGEPAIRS__EXTRACOLUMNS]
             [results.KEY__EXTRACOLUMNS__BUILDER]]
      builder_expectations = builder_dict.get(gm_json.JSONKEY_EXPECTEDRESULTS)
      if not builder_expectations:
        builder_expectations = {}
        builder_dict[gm_json.JSONKEY_EXPECTEDRESULTS] = builder_expectations
      builder_expectations[image_name] = new_expectations
    ExpectationComparisons._write_dicts_to_root(
        expected_builder_dicts, self._expected_root)

  @staticmethod
  def _write_dicts_to_root(meta_dict, root, pattern='*.json'):
    """Write all per-builder dictionaries within meta_dict to files under
    the root path.

    Security note: this will only write to files that already exist within
    the root path (as found by os.walk() within root), so we don't need to
    worry about malformed content writing to disk outside of root.
    However, the data written to those files is not double-checked, so it
    could contain poisonous data.

    Args:
      meta_dict: a builder-keyed meta-dictionary containing all the JSON
                 dictionaries we want to write out
      root: path to root of directory tree within which to write files
      pattern: which files to write within root (fnmatch-style pattern)

    Raises:
      IOError if root does not refer to an existing directory
      KeyError if the set of per-builder dictionaries written out was
               different than expected
    """
    if not os.path.isdir(root):
      raise IOError('no directory found at path %s' % root)
    actual_builders_written = []
    for dirpath, _, filenames in os.walk(root):
      for matching_filename in fnmatch.filter(filenames, pattern):
        builder = os.path.basename(dirpath)
        per_builder_dict = meta_dict.get(builder)
        if per_builder_dict is not None:
          fullpath = os.path.join(dirpath, matching_filename)
          gm_json.WriteToFile(per_builder_dict, fullpath)
          actual_builders_written.append(builder)

    # Check: did we write out the set of per-builder dictionaries we
    # expected to?
    expected_builders_written = sorted(meta_dict.keys())
    actual_builders_written.sort()
    if expected_builders_written != actual_builders_written:
      raise KeyError(
          'expected to write dicts for builders %s, but actually wrote them '
          'for builders %s' % (
              expected_builders_written, actual_builders_written))

  def _load_actual_and_expected(self):
    """Loads the results of all tests, across all builders (based on the
    files within self._actuals_root and self._expected_root),
    and stores them in self._results.
    """
    logging.info('Reading actual-results JSON files from %s...' %
                 self._actuals_root)
    actual_builder_dicts = self._read_builder_dicts_from_root(
        self._actuals_root)
    logging.info('Reading expected-results JSON files from %s...' %
                 self._expected_root)
    expected_builder_dicts = self._read_builder_dicts_from_root(
        self._expected_root)

    all_image_pairs = imagepairset.ImagePairSet(
        descriptions=IMAGEPAIR_SET_DESCRIPTIONS,
        diff_base_url=self._diff_base_url)
    failing_image_pairs = imagepairset.ImagePairSet(
        descriptions=IMAGEPAIR_SET_DESCRIPTIONS,
        diff_base_url=self._diff_base_url)

    # Override settings for columns that should be filtered using freeform text.
    for column_id in FREEFORM_COLUMN_IDS:
      factory = column.ColumnHeaderFactory(
          header_text=column_id, use_freeform_filter=True)
      all_image_pairs.set_column_header_factory(
          column_id=column_id, column_header_factory=factory)
      failing_image_pairs.set_column_header_factory(
          column_id=column_id, column_header_factory=factory)

    all_image_pairs.ensure_extra_column_values_in_summary(
        column_id=results.KEY__EXTRACOLUMNS__RESULT_TYPE, values=[
            results.KEY__RESULT_TYPE__FAILED,
            results.KEY__RESULT_TYPE__FAILUREIGNORED,
            results.KEY__RESULT_TYPE__NOCOMPARISON,
            results.KEY__RESULT_TYPE__SUCCEEDED,
        ])
    failing_image_pairs.ensure_extra_column_values_in_summary(
        column_id=results.KEY__EXTRACOLUMNS__RESULT_TYPE, values=[
            results.KEY__RESULT_TYPE__FAILED,
            results.KEY__RESULT_TYPE__FAILUREIGNORED,
            results.KEY__RESULT_TYPE__NOCOMPARISON,
        ])

    # Only consider builders we have both expected and actual results for.
    # Fixes http://skbug.com/2486 ('rebaseline_server shows actual results
    # (but not expectations) for Test-Ubuntu12-ShuttleA-NoGPU-x86_64-Debug
    # builder')
    actual_builder_set = set(actual_builder_dicts.keys())
    expected_builder_set = set(expected_builder_dicts.keys())
    builders = sorted(actual_builder_set.intersection(expected_builder_set))

    num_builders = len(builders)
    builder_num = 0
    for builder in builders:
      builder_num += 1
      logging.info('Generating pixel diffs for builder #%d of %d, "%s"...' %
                   (builder_num, num_builders, builder))
      actual_results_for_this_builder = (
          actual_builder_dicts[builder][gm_json.JSONKEY_ACTUALRESULTS])
      for result_type in sorted(actual_results_for_this_builder.keys()):
        results_of_this_type = actual_results_for_this_builder[result_type]
        if not results_of_this_type:
          continue
        for image_name in sorted(results_of_this_type.keys()):
          (test, config) = results.IMAGE_FILENAME_RE.match(image_name).groups()
          actual_image_relative_url = (
              ExpectationComparisons._create_relative_url(
                  hashtype_and_digest=results_of_this_type[image_name],
                  test_name=test))

          # Default empty expectations; overwrite these if we find any real ones
          expectations_per_test = None
          expected_image_relative_url = None
          expectations_dict = None
          try:
            expectations_per_test = (
                expected_builder_dicts
                [builder][gm_json.JSONKEY_EXPECTEDRESULTS][image_name])
            # TODO(epoger): assumes a single allowed digest per test, which is
            # fine; see https://code.google.com/p/skia/issues/detail?id=1787
            expected_image_hashtype_and_digest = (
                expectations_per_test
                [gm_json.JSONKEY_EXPECTEDRESULTS_ALLOWEDDIGESTS][0])
            expected_image_relative_url = (
                ExpectationComparisons._create_relative_url(
                    hashtype_and_digest=expected_image_hashtype_and_digest,
                    test_name=test))
            expectations_dict = {}
            for field in EXPECTATION_FIELDS_PASSED_THRU_VERBATIM:
              expectations_dict[field] = expectations_per_test.get(field)
          except (KeyError, TypeError):
            # There are several cases in which we would expect to find
            # no expectations for a given test:
            #
            # 1. result_type == NOCOMPARISON
            #   There are no expectations for this test yet!
            #
            # 2. alternate rendering mode failures (e.g. serialized)
            #   In cases like
            #   https://code.google.com/p/skia/issues/detail?id=1684
            #   ('tileimagefilter GM test failing in serialized render mode'),
            #   the gm-actuals will list a failure for the alternate
            #   rendering mode even though we don't have explicit expectations
            #   for the test (the implicit expectation is that it must
            #   render the same in all rendering modes).
            #
            # Don't log type 1, because it is common.
            # Log other types, because they are rare and we should know about
            # them, but don't throw an exception, because we need to keep our
            # tools working in the meanwhile!
            if result_type != results.KEY__RESULT_TYPE__NOCOMPARISON:
              logging.warning('No expectations found for test: %s' % {
                  results.KEY__EXTRACOLUMNS__BUILDER: builder,
                  results.KEY__EXTRACOLUMNS__RESULT_TYPE: result_type,
                  'image_name': image_name,
                  })

          # If this test was recently rebaselined, it will remain in
          # the 'failed' set of actuals until all the bots have
          # cycled (although the expectations have indeed been set
          # from the most recent actuals).  Treat these as successes
          # instead of failures.
          #
          # TODO(epoger): Do we need to do something similar in
          # other cases, such as when we have recently marked a test
          # as ignoreFailure but it still shows up in the 'failed'
          # category?  Maybe we should not rely on the result_type
          # categories recorded within the gm_actuals AT ALL, and
          # instead evaluate the result_type ourselves based on what
          # we see in expectations vs actual checksum?
          if expected_image_relative_url == actual_image_relative_url:
            updated_result_type = results.KEY__RESULT_TYPE__SUCCEEDED
          elif ((result_type == results.KEY__RESULT_TYPE__FAILED) and
                (test in self._ignore_failures_on_these_tests)):
            updated_result_type = results.KEY__RESULT_TYPE__FAILUREIGNORED
          else:
            updated_result_type = result_type
          extra_columns_dict = {
              results.KEY__EXTRACOLUMNS__RESULT_TYPE: updated_result_type,
              results.KEY__EXTRACOLUMNS__BUILDER: builder,
              results.KEY__EXTRACOLUMNS__TEST: test,
              results.KEY__EXTRACOLUMNS__CONFIG: config,
          }
          try:
            image_pair = imagepair.ImagePair(
                image_diff_db=self._image_diff_db,
                imageA_base_url=gm_json.GM_ACTUALS_ROOT_HTTP_URL,
                imageB_base_url=gm_json.GM_ACTUALS_ROOT_HTTP_URL,
                imageA_relative_url=expected_image_relative_url,
                imageB_relative_url=actual_image_relative_url,
                expectations=expectations_dict,
                extra_columns=extra_columns_dict)
            all_image_pairs.add_image_pair(image_pair)
            if updated_result_type != results.KEY__RESULT_TYPE__SUCCEEDED:
              failing_image_pairs.add_image_pair(image_pair)
          except Exception:
            logging.exception('got exception while creating new ImagePair')

    # pylint: disable=W0201
    self._results = {
      results.KEY__HEADER__RESULTS_ALL: all_image_pairs.as_dict(
          column_ids_in_order=ORDERED_COLUMN_IDS),
      results.KEY__HEADER__RESULTS_FAILURES: failing_image_pairs.as_dict(
          column_ids_in_order=ORDERED_COLUMN_IDS),
    }


def main():
  logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                      datefmt='%m/%d/%Y %H:%M:%S',
                      level=logging.INFO)
  parser = argparse.ArgumentParser()
  parser.add_argument(
      '--actuals', default=results.DEFAULT_ACTUALS_DIR,
      help='Directory containing all actual-result JSON files; defaults to '
      '\'%(default)s\' .')
  parser.add_argument(
      '--expectations', default=DEFAULT_EXPECTATIONS_DIR,
      help='Directory containing all expected-result JSON files; defaults to '
      '\'%(default)s\' .')
  parser.add_argument(
      '--ignore-failures-file', default=DEFAULT_IGNORE_FAILURES_FILE,
      help='If a file with this name is found within the EXPECTATIONS dir, '
      'ignore failures for any tests listed in the file; defaults to '
      '\'%(default)s\' .')
  parser.add_argument(
      '--outfile', required=True,
      help='File to write result summary into, in JSON format.')
  parser.add_argument(
      '--results', default=results.KEY__HEADER__RESULTS_FAILURES,
      help='Which result types to include. Defaults to \'%(default)s\'; '
      'must be one of ' +
      str([results.KEY__HEADER__RESULTS_FAILURES,
           results.KEY__HEADER__RESULTS_ALL]))
  parser.add_argument(
      '--workdir', default=results.DEFAULT_GENERATED_IMAGES_ROOT,
      help='Directory within which to download images and generate diffs; '
      'defaults to \'%(default)s\' .')
  args = parser.parse_args()
  image_diff_db = imagediffdb.ImageDiffDB(storage_root=args.workdir)
  results_obj = ExpectationComparisons(
      image_diff_db=image_diff_db,
      actuals_root=args.actuals,
      expected_root=args.expectations,
      ignore_failures_file=args.ignore_failures_file)
  gm_json.WriteToFile(
      results_obj.get_packaged_results_of_type(results_type=args.results),
      args.outfile)


if __name__ == '__main__':
  main()
