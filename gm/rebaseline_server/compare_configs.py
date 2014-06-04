#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Compare GM results for two configs, across all builders.
"""

# System-level imports
import argparse
import fnmatch
import json
import logging
import re
import time

# Imports from within Skia
import fix_pythonpath  # must do this first
from pyutils import url_utils
import gm_json
import imagediffdb
import imagepair
import imagepairset
import results


class ConfigComparisons(results.BaseComparisons):
  """Loads results from two different configurations into an ImagePairSet.

  Loads actual and expected results from all builders, except for those skipped
  by _ignore_builder().
  """

  def __init__(self, configs, actuals_root=results.DEFAULT_ACTUALS_DIR,
               generated_images_root=results.DEFAULT_GENERATED_IMAGES_ROOT,
               diff_base_url=None, builder_regex_list=None):
    """
    Args:
      configs: (string, string) tuple; pair of configs to compare
      actuals_root: root directory containing all actual-results.json files
      generated_images_root: directory within which to create all pixel diffs;
          if this directory does not yet exist, it will be created
      diff_base_url: base URL within which the client should look for diff
          images; if not specified, defaults to a "file:///" URL representation
          of generated_images_root
      builder_regex_list: List of regular expressions specifying which builders
          we will process. If None, process all builders.
    """
    time_start = int(time.time())
    if builder_regex_list != None:
      self.set_match_builders_pattern_list(builder_regex_list)
    self._image_diff_db = imagediffdb.ImageDiffDB(generated_images_root)
    self._diff_base_url = (
        diff_base_url or
        url_utils.create_filepath_url(generated_images_root))
    self._actuals_root = actuals_root
    self._load_config_pairs(configs)
    self._timestamp = int(time.time())
    logging.info('Results complete; took %d seconds.' %
                 (self._timestamp - time_start))

  def _load_config_pairs(self, configs):
    """Loads the results of all tests, across all builders (based on the
    files within self._actuals_root), compares them across two configs,
    and stores the summary in self._results.

    Args:
      configs: tuple of strings; pair of configs to compare
    """
    logging.info('Reading actual-results JSON files from %s...' %
                 self._actuals_root)
    actual_builder_dicts = self._read_builder_dicts_from_root(
        self._actuals_root)
    configA, configB = configs
    logging.info('Comparing configs %s and %s...' % (configA, configB))

    all_image_pairs = imagepairset.ImagePairSet(
        descriptions=configs,
        diff_base_url=self._diff_base_url)
    failing_image_pairs = imagepairset.ImagePairSet(
        descriptions=configs,
        diff_base_url=self._diff_base_url)

    all_image_pairs.ensure_extra_column_values_in_summary(
        column_id=results.KEY__EXTRACOLUMNS__RESULT_TYPE, values=[
            results.KEY__RESULT_TYPE__FAILED,
            results.KEY__RESULT_TYPE__NOCOMPARISON,
            results.KEY__RESULT_TYPE__SUCCEEDED,
        ])
    failing_image_pairs.ensure_extra_column_values_in_summary(
        column_id=results.KEY__EXTRACOLUMNS__RESULT_TYPE, values=[
            results.KEY__RESULT_TYPE__FAILED,
            results.KEY__RESULT_TYPE__NOCOMPARISON,
        ])

    builders = sorted(actual_builder_dicts.keys())
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

        tests_found = set()
        for image_name in sorted(results_of_this_type.keys()):
          (test, config) = results.IMAGE_FILENAME_RE.match(image_name).groups()
          tests_found.add(test)

        for test in tests_found:
          # Get image_relative_url (or None) for each of configA, configB
          image_name_A = results.IMAGE_FILENAME_FORMATTER % (test, configA)
          configA_image_relative_url = ConfigComparisons._create_relative_url(
              hashtype_and_digest=results_of_this_type.get(image_name_A),
              test_name=test)
          image_name_B = results.IMAGE_FILENAME_FORMATTER % (test, configB)
          configB_image_relative_url = ConfigComparisons._create_relative_url(
              hashtype_and_digest=results_of_this_type.get(image_name_B),
              test_name=test)

          # If we have images for at least one of these two configs,
          # add them to our list.
          if configA_image_relative_url or configB_image_relative_url:
            if configA_image_relative_url == configB_image_relative_url:
              result_type = results.KEY__RESULT_TYPE__SUCCEEDED
            elif not configA_image_relative_url:
              result_type = results.KEY__RESULT_TYPE__NOCOMPARISON
            elif not configB_image_relative_url:
              result_type = results.KEY__RESULT_TYPE__NOCOMPARISON
            else:
              result_type = results.KEY__RESULT_TYPE__FAILED

            extra_columns_dict = {
                results.KEY__EXTRACOLUMNS__RESULT_TYPE: result_type,
                results.KEY__EXTRACOLUMNS__BUILDER: builder,
                results.KEY__EXTRACOLUMNS__TEST: test,
                # TODO(epoger): Right now, the client UI crashes if it receives
                # results that do not include a 'config' column.
                # Until we fix that, keep the client happy.
                results.KEY__EXTRACOLUMNS__CONFIG: 'TODO',
            }

            try:
              image_pair = imagepair.ImagePair(
                  image_diff_db=self._image_diff_db,
                  base_url=gm_json.GM_ACTUALS_ROOT_HTTP_URL,
                  imageA_relative_url=configA_image_relative_url,
                  imageB_relative_url=configB_image_relative_url,
                  extra_columns=extra_columns_dict)
              all_image_pairs.add_image_pair(image_pair)
              if result_type != results.KEY__RESULT_TYPE__SUCCEEDED:
                failing_image_pairs.add_image_pair(image_pair)
            except (KeyError, TypeError):
              logging.exception(
                  'got exception while creating ImagePair for image_name '
                  '"%s", builder "%s"' % (image_name, builder))

    self._results = {
      results.KEY__HEADER__RESULTS_ALL: all_image_pairs.as_dict(),
      results.KEY__HEADER__RESULTS_FAILURES: failing_image_pairs.as_dict(),
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
      'config', nargs=2,
      help='Two configurations to compare (8888, gpu, etc.).')
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
  results_obj = ConfigComparisons(configs=args.config,
                                  actuals_root=args.actuals,
                                  generated_images_root=args.workdir)
  gm_json.WriteToFile(
      results_obj.get_packaged_results_of_type(results_type=args.results),
      args.outfile)


if __name__ == '__main__':
  main()
