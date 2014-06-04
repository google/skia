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
import time

# Imports from within Skia
import fix_pythonpath  # must do this first
from pyutils import url_utils
import gm_json
import imagediffdb
import imagepair
import imagepairset
import results

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
        url_utils.create_filepath_url(generated_images_root))
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
    subdirA_dicts = self._read_dicts_from_root(
        os.path.join(actuals_root, subdirA))
    subdirB_dicts = self._read_dicts_from_root(
        os.path.join(actuals_root, subdirB))
    logging.info('Comparing subdirs %s and %s...' % (subdirA, subdirB))

    all_image_pairs = imagepairset.ImagePairSet(
        descriptions=subdirs,
        diff_base_url=self._diff_base_url)
    failing_image_pairs = imagepairset.ImagePairSet(
        descriptions=subdirs,
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

    common_dict_paths = sorted(set(subdirA_dicts.keys() + subdirB_dicts.keys()))
    num_common_dict_paths = len(common_dict_paths)
    dict_num = 0
    for dict_path in common_dict_paths:
      dict_num += 1
      logging.info('Generating pixel diffs for dict #%d of %d, "%s"...' %
                   (dict_num, num_common_dict_paths, dict_path))
      dictA = subdirA_dicts[dict_path]
      dictB = subdirB_dicts[dict_path]
      self._validate_dict_version(dictA)
      self._validate_dict_version(dictB)
      dictA_results = dictA[gm_json.JSONKEY_ACTUALRESULTS]
      dictB_results = dictB[gm_json.JSONKEY_ACTUALRESULTS]
      skp_names = sorted(set(dictA_results.keys() + dictB_results.keys()))
      for skp_name in skp_names:
        imagepairs_for_this_skp = []

        whole_image_A = RenderedPicturesComparisons.get_multilevel(
            dictA_results, skp_name, gm_json.JSONKEY_SOURCE_WHOLEIMAGE)
        whole_image_B = RenderedPicturesComparisons.get_multilevel(
            dictB_results, skp_name, gm_json.JSONKEY_SOURCE_WHOLEIMAGE)
        imagepairs_for_this_skp.append(self._create_image_pair(
            test=skp_name, config=gm_json.JSONKEY_SOURCE_WHOLEIMAGE,
            image_dict_A=whole_image_A, image_dict_B=whole_image_B))

        tiled_images_A = RenderedPicturesComparisons.get_multilevel(
            dictA_results, skp_name, gm_json.JSONKEY_SOURCE_TILEDIMAGES)
        tiled_images_B = RenderedPicturesComparisons.get_multilevel(
            dictB_results, skp_name, gm_json.JSONKEY_SOURCE_TILEDIMAGES)
        # TODO(epoger): Report an error if we find tiles for A but not B?
        if tiled_images_A and tiled_images_B:
          # TODO(epoger): Report an error if we find a different number of tiles
          # for A and B?
          num_tiles = len(tiled_images_A)
          for tile_num in range(num_tiles):
            imagepairs_for_this_skp.append(self._create_image_pair(
                test=skp_name,
                config='%s-%d' % (gm_json.JSONKEY_SOURCE_TILEDIMAGES, tile_num),
                image_dict_A=tiled_images_A[tile_num],
                image_dict_B=tiled_images_B[tile_num]))

        for imagepair in imagepairs_for_this_skp:
          if imagepair:
            all_image_pairs.add_image_pair(imagepair)
            result_type = imagepair.extra_columns_dict\
                [results.KEY__EXTRACOLUMNS__RESULT_TYPE]
            if result_type != results.KEY__RESULT_TYPE__SUCCEEDED:
              failing_image_pairs.add_image_pair(imagepair)

    self._results = {
      results.KEY__HEADER__RESULTS_ALL: all_image_pairs.as_dict(),
      results.KEY__HEADER__RESULTS_FAILURES: failing_image_pairs.as_dict(),
    }

  def _validate_dict_version(self, result_dict):
    """Raises Exception if the dict is not the type/version we know how to read.

    Args:
      result_dict: dictionary holding output of render_pictures
    """
    expected_header_type = 'ChecksummedImages'
    expected_header_revision = 1

    header = result_dict[gm_json.JSONKEY_HEADER]
    header_type = header[gm_json.JSONKEY_HEADER_TYPE]
    if header_type != expected_header_type:
      raise Exception('expected header_type "%s", but got "%s"' % (
          expected_header_type, header_type))
    header_revision = header[gm_json.JSONKEY_HEADER_REVISION]
    if header_revision != expected_header_revision:
      raise Exception('expected header_revision %d, but got %d' % (
          expected_header_revision, header_revision))

  def _create_image_pair(self, test, config, image_dict_A, image_dict_B):
    """Creates an ImagePair object for this pair of images.

    Args:
      test: string; name of the test
      config: string; name of the config
      image_dict_A: dict with JSONKEY_IMAGE_* keys, or None if no image
      image_dict_B: dict with JSONKEY_IMAGE_* keys, or None if no image

    Returns:
      An ImagePair object, or None if both image_dict_A and image_dict_B are
      None.
    """
    if (not image_dict_A) and (not image_dict_B):
      return None

    def _checksum_and_relative_url(dic):
      if dic:
        return ((dic[gm_json.JSONKEY_IMAGE_CHECKSUMALGORITHM],
                 dic[gm_json.JSONKEY_IMAGE_CHECKSUMVALUE]),
                dic[gm_json.JSONKEY_IMAGE_FILEPATH])
      else:
        return None, None

    imageA_checksum, imageA_relative_url = _checksum_and_relative_url(
        image_dict_A)
    imageB_checksum, imageB_relative_url = _checksum_and_relative_url(
        image_dict_B)

    if not imageA_checksum:
      result_type = results.KEY__RESULT_TYPE__NOCOMPARISON
    elif not imageB_checksum:
      result_type = results.KEY__RESULT_TYPE__NOCOMPARISON
    elif imageA_checksum == imageB_checksum:
      result_type = results.KEY__RESULT_TYPE__SUCCEEDED
    else:
      result_type = results.KEY__RESULT_TYPE__FAILED

    extra_columns_dict = {
        results.KEY__EXTRACOLUMNS__CONFIG: config,
        results.KEY__EXTRACOLUMNS__RESULT_TYPE: result_type,
        results.KEY__EXTRACOLUMNS__TEST: test,
        # TODO(epoger): Right now, the client UI crashes if it receives
        # results that do not include this column.
        # Until we fix that, keep the client happy.
        results.KEY__EXTRACOLUMNS__BUILDER: 'TODO',
    }

    try:
      return imagepair.ImagePair(
          image_diff_db=self._image_diff_db,
          base_url=self._image_base_url,
          imageA_relative_url=imageA_relative_url,
          imageB_relative_url=imageB_relative_url,
          extra_columns=extra_columns_dict)
    except (KeyError, TypeError):
      logging.exception(
          'got exception while creating ImagePair for'
          ' test="%s", config="%s", urlPair=("%s","%s")' % (
              test, config, imageA_relative_url, imageB_relative_url))
      return None


# TODO(epoger): Add main() so this can be called by vm_run_skia_try.sh
