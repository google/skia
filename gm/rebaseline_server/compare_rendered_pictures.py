#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Compare results of two render_pictures runs.
"""

# System-level imports
import logging
import os
import re
import sys
import time

# Imports from within Skia
#
# TODO(epoger): Once we move the create_filepath_url() function out of
# download_actuals into a shared utility module, we won't need to import
# download_actuals anymore.
#
# We need to add the 'gm' directory, so that we can import gm_json.py within
# that directory.  That script allows us to parse the actual-results.json file
# written out by the GM tool.
# Make sure that the 'gm' dir is in the PYTHONPATH, but add it at the *end*
# so any dirs that are already in the PYTHONPATH will be preferred.
PARENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
GM_DIRECTORY = os.path.dirname(PARENT_DIRECTORY)
TRUNK_DIRECTORY = os.path.dirname(GM_DIRECTORY)
if GM_DIRECTORY not in sys.path:
  sys.path.append(GM_DIRECTORY)
import download_actuals
import gm_json
import imagediffdb
import imagepair
import imagepairset
import results

# Characters we don't want popping up just anywhere within filenames.
DISALLOWED_FILEPATH_CHAR_REGEX = re.compile('[^\w\-]')

# URL under which all render_pictures images can be found in Google Storage.
# TODO(epoger): Move this default value into
# https://skia.googlesource.com/buildbot/+/master/site_config/global_variables.json
DEFAULT_IMAGE_BASE_URL = 'http://chromium-skia-gm.commondatastorage.googleapis.com/render_pictures/images'


class RenderedPicturesComparisons(results.BaseComparisons):
  """Loads results from two different render_pictures runs into an ImagePairSet.
  """

  def __init__(self, subdirs, actuals_root,
               generated_images_root=results.DEFAULT_GENERATED_IMAGES_ROOT,
               image_base_url=DEFAULT_IMAGE_BASE_URL,
               diff_base_url=None):
    """
    Args:
      actuals_root: root directory containing all render_pictures-generated
          JSON files
      subdirs: (string, string) tuple; pair of subdirectories within
          actuals_root to compare
      generated_images_root: directory within which to create all pixel diffs;
          if this directory does not yet exist, it will be created
      image_base_url: URL under which all render_pictures result images can
          be found; this will be used to read images for comparison within
          this code, and included in the ImagePairSet so its consumers know
          where to download the images from
      diff_base_url: base URL within which the client should look for diff
          images; if not specified, defaults to a "file:///" URL representation
          of generated_images_root
    """
    time_start = int(time.time())
    self._image_diff_db = imagediffdb.ImageDiffDB(generated_images_root)
    self._image_base_url = image_base_url
    self._diff_base_url = (
        diff_base_url or
        download_actuals.create_filepath_url(generated_images_root))
    self._load_result_pairs(actuals_root, subdirs)
    self._timestamp = int(time.time())
    logging.info('Results complete; took %d seconds.' %
                 (self._timestamp - time_start))

  def _load_result_pairs(self, actuals_root, subdirs):
    """Loads all JSON files found within two subdirs in actuals_root,
    compares across those two subdirs, and stores the summary in self._results.

    Args:
      actuals_root: root directory containing all render_pictures-generated
          JSON files
      subdirs: (string, string) tuple; pair of subdirectories within
          actuals_root to compare
    """
    logging.info(
        'Reading actual-results JSON files from %s subdirs within %s...' % (
            subdirs, actuals_root))
    subdirA, subdirB = subdirs
    subdirA_builder_dicts = self._read_dicts_from_root(
        os.path.join(actuals_root, subdirA))
    subdirB_builder_dicts = self._read_dicts_from_root(
        os.path.join(actuals_root, subdirB))
    logging.info('Comparing subdirs %s and %s...' % (subdirA, subdirB))

    all_image_pairs = imagepairset.ImagePairSet(
        descriptions=subdirs,
        diff_base_url=self._diff_base_url)
    failing_image_pairs = imagepairset.ImagePairSet(
        descriptions=subdirs,
        diff_base_url=self._diff_base_url)

    all_image_pairs.ensure_extra_column_values_in_summary(
        column_id=results.KEY__EXTRACOLUMN__RESULT_TYPE, values=[
            results.KEY__RESULT_TYPE__FAILED,
            results.KEY__RESULT_TYPE__NOCOMPARISON,
            results.KEY__RESULT_TYPE__SUCCEEDED,
        ])
    failing_image_pairs.ensure_extra_column_values_in_summary(
        column_id=results.KEY__EXTRACOLUMN__RESULT_TYPE, values=[
            results.KEY__RESULT_TYPE__FAILED,
            results.KEY__RESULT_TYPE__NOCOMPARISON,
        ])

    builders = sorted(set(subdirA_builder_dicts.keys() +
                          subdirB_builder_dicts.keys()))
    num_builders = len(builders)
    builder_num = 0
    for builder in builders:
      builder_num += 1
      logging.info('Generating pixel diffs for builder #%d of %d, "%s"...' %
                   (builder_num, num_builders, builder))
      # TODO(epoger): This will fail if we have results for this builder in
      # subdirA but not subdirB (or vice versa).
      subdirA_results = results.BaseComparisons.combine_subdicts(
          subdirA_builder_dicts[builder][gm_json.JSONKEY_ACTUALRESULTS])
      subdirB_results = results.BaseComparisons.combine_subdicts(
          subdirB_builder_dicts[builder][gm_json.JSONKEY_ACTUALRESULTS])
      image_names = sorted(set(subdirA_results.keys() +
                               subdirB_results.keys()))
      for image_name in image_names:
        # The image name may contain funny characters or be ridiculously long
        # (see https://code.google.com/p/skia/issues/detail?id=2344#c10 ),
        # so make sure we sanitize it before using it in a URL path.
        #
        # TODO(epoger): Rather than sanitizing/truncating the image name here,
        # do it in render_pictures instead.
        # Reason: we will need to be consistent in applying this rule, so that
        # the process which uploads the files to GS using these paths will
        # match the paths created by downstream processes.
        # So, we should make render_pictures write out images to paths that are
        # "ready to upload" to Google Storage, like gm does.
        sanitized_test_name = DISALLOWED_FILEPATH_CHAR_REGEX.sub(
            '_', image_name)[:30]

        subdirA_image_relative_url = (
            results.BaseComparisons._create_relative_url(
                hashtype_and_digest=subdirA_results.get(image_name),
                test_name=sanitized_test_name))
        subdirB_image_relative_url = (
            results.BaseComparisons._create_relative_url(
                hashtype_and_digest=subdirB_results.get(image_name),
                test_name=sanitized_test_name))

        # If we have images for at least one of these two subdirs,
        # add them to our list.
        if subdirA_image_relative_url or subdirB_image_relative_url:
          if subdirA_image_relative_url == subdirB_image_relative_url:
            result_type = results.KEY__RESULT_TYPE__SUCCEEDED
          elif not subdirA_image_relative_url:
            result_type = results.KEY__RESULT_TYPE__NOCOMPARISON
          elif not subdirB_image_relative_url:
            result_type = results.KEY__RESULT_TYPE__NOCOMPARISON
          else:
            result_type = results.KEY__RESULT_TYPE__FAILED

        extra_columns_dict = {
            results.KEY__EXTRACOLUMN__RESULT_TYPE: result_type,
            results.KEY__EXTRACOLUMN__BUILDER: builder,
            results.KEY__EXTRACOLUMN__TEST: image_name,
            # TODO(epoger): Right now, the client UI crashes if it receives
            # results that do not include a 'config' column.
            # Until we fix that, keep the client happy.
            results.KEY__EXTRACOLUMN__CONFIG: 'TODO',
        }

        try:
          image_pair = imagepair.ImagePair(
              image_diff_db=self._image_diff_db,
              base_url=self._image_base_url,
              imageA_relative_url=subdirA_image_relative_url,
              imageB_relative_url=subdirB_image_relative_url,
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


# TODO(epoger): Add main() so this can be called by vm_run_skia_try.sh
