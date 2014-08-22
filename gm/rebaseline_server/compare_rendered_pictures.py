#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Compare results of two render_pictures runs.

TODO(epoger): Start using this module to compare ALL images (whether they
were generated from GMs or SKPs), and rename it accordingly.
"""

# System-level imports
import logging
import os
import shutil
import subprocess
import tempfile
import time

# Must fix up PYTHONPATH before importing from within Skia
import rs_fixpypath  # pylint: disable=W0611

# Imports from within Skia
from py.utils import git_utils
from py.utils import gs_utils
from py.utils import url_utils
import buildbot_globals
import column
import gm_json
import imagediffdb
import imagepair
import imagepairset
import results

# URL under which all render_pictures images can be found in Google Storage.
#
# TODO(epoger): In order to allow live-view of GMs and other images, read this
# from the input summary files, or allow the caller to set it within the
# GET_live_results call.
DEFAULT_IMAGE_BASE_GS_URL = 'gs://' + buildbot_globals.Get('skp_images_bucket')

# Column descriptors, and display preferences for them.
COLUMN__RESULT_TYPE = results.KEY__EXTRACOLUMNS__RESULT_TYPE
COLUMN__SOURCE_SKP = 'sourceSkpFile'
COLUMN__TILED_OR_WHOLE = 'tiledOrWhole'
COLUMN__TILENUM = 'tilenum'
COLUMN__BUILDER_A = 'builderA'
COLUMN__RENDER_MODE_A = 'renderModeA'
COLUMN__BUILDER_B = 'builderB'
COLUMN__RENDER_MODE_B = 'renderModeB'
# Known values for some of those columns.
COLUMN__TILED_OR_WHOLE__TILED = 'tiled'
COLUMN__TILED_OR_WHOLE__WHOLE = 'whole'

FREEFORM_COLUMN_IDS = [
    COLUMN__SOURCE_SKP,
    COLUMN__TILENUM,
]
ORDERED_COLUMN_IDS = [
    COLUMN__RESULT_TYPE,
    COLUMN__SOURCE_SKP,
    COLUMN__TILED_OR_WHOLE,
    COLUMN__TILENUM,
    COLUMN__BUILDER_A,
    COLUMN__RENDER_MODE_A,
    COLUMN__BUILDER_B,
    COLUMN__RENDER_MODE_B,
]

# A special "repo:" URL type that we use to refer to Skia repo contents.
# (Useful for comparing against expectations files we store in our repo.)
REPO_URL_PREFIX = 'repo:'
REPO_BASEPATH = os.path.abspath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), os.pardir, os.pardir))

# Which sections within a JSON summary file can contain results.
ALLOWED_SECTION_NAMES = [
    gm_json.JSONKEY_ACTUALRESULTS,
    gm_json.JSONKEY_EXPECTEDRESULTS,
]


class RenderedPicturesComparisons(results.BaseComparisons):
  """Loads results from multiple render_pictures runs into an ImagePairSet.
  """

  def __init__(self,
               setA_dir, setB_dir,
               setA_section, setB_section,
               image_diff_db,
               image_base_gs_url=DEFAULT_IMAGE_BASE_GS_URL, diff_base_url=None,
               setA_label=None, setB_label=None,
               gs=None, truncate_results=False, prefetch_only=False,
               download_all_images=False):
    """Constructor: downloads images and generates diffs.

    Once the object has been created (which may take a while), you can call its
    get_packaged_results_of_type() method to quickly retrieve the results...
    unless you have set prefetch_only to True, in which case we will
    asynchronously warm up the ImageDiffDB cache but not fill in self._results.

    Args:
      setA_dir: root directory to copy all JSON summaries from, and to use as
          setA within the comparisons. This directory may be specified as a
          gs:// URL, special "repo:" URL, or local filepath.
      setB_dir: root directory to copy all JSON summaries from, and to use as
          setB within the comparisons. This directory may be specified as a
          gs:// URL, special "repo:" URL, or local filepath.
      setA_section: which section within setA to examine; must be one of
          ALLOWED_SECTION_NAMES
      setB_section: which section within setB to examine; must be one of
          ALLOWED_SECTION_NAMES
      image_diff_db: ImageDiffDB instance
      image_base_gs_url: "gs://" URL pointing at the Google Storage bucket/dir
          under which all render_pictures result images can
          be found; this will be used to read images for comparison within
          this code, and included in the ImagePairSet (as an HTTP URL) so its
          consumers know where to download the images from
      diff_base_url: base URL within which the client should look for diff
          images; if not specified, defaults to a "file:///" URL representation
          of image_diff_db's storage_root
      setA_label: description to use for results in setA; if None, will be
          set to a reasonable default
      setB_label: description to use for results in setB; if None, will be
          set to a reasonable default
      gs: instance of GSUtils object we can use to download summary files
      truncate_results: FOR MANUAL TESTING: if True, truncate the set of images
          we process, to speed up testing.
      prefetch_only: if True, return the new object as quickly as possible
          with empty self._results (just queue up all the files to process,
          don't wait around for them to be processed and recorded); otherwise,
          block until the results have been assembled and recorded in
          self._results.
      download_all_images: if True, download all images, even if we don't
          need them to generate diffs.  This will take much longer to complete,
          but is useful for warming up the bitmap cache on local disk.
    """
    super(RenderedPicturesComparisons, self).__init__()
    self._image_diff_db = image_diff_db
    self._image_base_gs_url = image_base_gs_url
    self._diff_base_url = (
        diff_base_url or
        url_utils.create_filepath_url(image_diff_db.storage_root))
    self._gs = gs
    self.truncate_results = truncate_results
    self._prefetch_only = prefetch_only
    self._download_all_images = download_all_images

    # If we are comparing two different section types, we can use those
    # as the default labels for setA and setB.
    if setA_section != setB_section:
      self._setA_label = setA_label or setA_section
      self._setB_label = setB_label or setB_section
    else:
      self._setA_label = setA_label or 'setA'
      self._setB_label = setB_label or 'setB'

    tempdir = tempfile.mkdtemp()
    try:
      setA_root = os.path.join(tempdir, 'setA')
      setB_root = os.path.join(tempdir, 'setB')
      # TODO(stephana): There is a potential race condition here... we copy
      # the contents out of the source_dir, and THEN we get the commithash
      # of source_dir.  If source_dir points at a git checkout, and that
      # checkout is updated (by a different thread/process) during this
      # operation, then the contents and commithash will be out of sync.
      self._copy_dir_contents(source_dir=setA_dir, dest_dir=setA_root)
      setA_repo_revision = self._get_repo_revision(source_dir=setA_dir)
      self._copy_dir_contents(source_dir=setB_dir, dest_dir=setB_root)
      setB_repo_revision = self._get_repo_revision(source_dir=setB_dir)

      self._setA_descriptions = {
          results.KEY__SET_DESCRIPTIONS__DIR: setA_dir,
          results.KEY__SET_DESCRIPTIONS__REPO_REVISION: setA_repo_revision,
          results.KEY__SET_DESCRIPTIONS__SECTION: setA_section,
      }
      self._setB_descriptions = {
          results.KEY__SET_DESCRIPTIONS__DIR: setB_dir,
          results.KEY__SET_DESCRIPTIONS__REPO_REVISION: setB_repo_revision,
          results.KEY__SET_DESCRIPTIONS__SECTION: setB_section,
      }

      time_start = int(time.time())
      self._results = self._load_result_pairs(
          setA_root=setA_root, setB_root=setB_root,
          setA_section=setA_section, setB_section=setB_section)
      if self._results:
        self._timestamp = int(time.time())
        logging.info('Number of download file collisions: %s' %
                     imagediffdb.global_file_collisions)
        logging.info('Results complete; took %d seconds.' %
                     (self._timestamp - time_start))
    finally:
      shutil.rmtree(tempdir)

  def _load_result_pairs(self, setA_root, setB_root,
                         setA_section, setB_section):
    """Loads all JSON image summaries from 2 directory trees and compares them.

    TODO(stephana): This method is only called from within __init__(); it might
    make more sense to just roll the content of this method into __init__().

    Args:
      setA_root: root directory containing JSON summaries of rendering results
      setB_root: root directory containing JSON summaries of rendering results
      setA_section: which section (gm_json.JSONKEY_ACTUALRESULTS or
          gm_json.JSONKEY_EXPECTEDRESULTS) to load from the summaries in setA
      setB_section: which section (gm_json.JSONKEY_ACTUALRESULTS or
          gm_json.JSONKEY_EXPECTEDRESULTS) to load from the summaries in setB

    Returns the summary of all image diff results (or None, depending on
    self._prefetch_only).
    """
    logging.info('Reading JSON image summaries from dirs %s and %s...' % (
        setA_root, setB_root))
    setA_dicts = self.read_dicts_from_root(setA_root)
    setB_dicts = self.read_dicts_from_root(setB_root)
    logging.info('Comparing summary dicts...')

    all_image_pairs = imagepairset.ImagePairSet(
        descriptions=(self._setA_label, self._setB_label),
        diff_base_url=self._diff_base_url)
    failing_image_pairs = imagepairset.ImagePairSet(
        descriptions=(self._setA_label, self._setB_label),
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
        column_id=COLUMN__RESULT_TYPE, values=[
            results.KEY__RESULT_TYPE__FAILED,
            results.KEY__RESULT_TYPE__NOCOMPARISON,
            results.KEY__RESULT_TYPE__SUCCEEDED,
        ])
    failing_image_pairs.ensure_extra_column_values_in_summary(
        column_id=COLUMN__RESULT_TYPE, values=[
            results.KEY__RESULT_TYPE__FAILED,
            results.KEY__RESULT_TYPE__NOCOMPARISON,
        ])

    logging.info('Starting to add imagepairs to queue.')
    self._image_diff_db.log_queue_size_if_changed(limit_verbosity=False)

    union_dict_paths = sorted(set(setA_dicts.keys() + setB_dicts.keys()))
    num_union_dict_paths = len(union_dict_paths)
    dict_num = 0
    for dict_path in union_dict_paths:
      dict_num += 1
      logging.info(
          'Asynchronously requesting pixel diffs for dict #%d of %d, "%s"...' %
          (dict_num, num_union_dict_paths, dict_path))

      dictA = self.get_default(setA_dicts, None, dict_path)
      self._validate_dict_version(dictA)
      dictA_results = self.get_default(dictA, {}, setA_section)

      dictB = self.get_default(setB_dicts, None, dict_path)
      self._validate_dict_version(dictB)
      dictB_results = self.get_default(dictB, {}, setB_section)

      image_A_base_url = self.get_default(
          setA_dicts, self._image_base_gs_url, dict_path,
          gm_json.JSONKEY_IMAGE_BASE_GS_URL)
      image_B_base_url = self.get_default(
          setB_dicts, self._image_base_gs_url, dict_path,
          gm_json.JSONKEY_IMAGE_BASE_GS_URL)

      # get the builders and render modes for each set
      builder_A     = self.get_default(dictA, None,
                        gm_json.JSONKEY_DESCRIPTIONS,
                        gm_json.JSONKEY_DESCRIPTIONS_BUILDER)
      render_mode_A = self.get_default(dictA, None,
                        gm_json.JSONKEY_DESCRIPTIONS,
                        gm_json.JSONKEY_DESCRIPTIONS_RENDER_MODE)
      builder_B     = self.get_default(dictB, None,
                        gm_json.JSONKEY_DESCRIPTIONS,
                        gm_json.JSONKEY_DESCRIPTIONS_BUILDER)
      render_mode_B = self.get_default(dictB, None,
                        gm_json.JSONKEY_DESCRIPTIONS,
                        gm_json.JSONKEY_DESCRIPTIONS_RENDER_MODE)

      skp_names = sorted(set(dictA_results.keys() + dictB_results.keys()))
      # Just for manual testing... truncate to an arbitrary subset.
      if self.truncate_results:
        skp_names = skp_names[1:3]
      for skp_name in skp_names:
        imagepairs_for_this_skp = []

        whole_image_A = self.get_default(
            dictA_results, None,
            skp_name, gm_json.JSONKEY_SOURCE_WHOLEIMAGE)
        whole_image_B = self.get_default(
            dictB_results, None,
            skp_name, gm_json.JSONKEY_SOURCE_WHOLEIMAGE)

        imagepairs_for_this_skp.append(self._create_image_pair(
            image_dict_A=whole_image_A, image_dict_B=whole_image_B,
            image_A_base_url=image_A_base_url,
            image_B_base_url=image_B_base_url,
            builder_A=builder_A, render_mode_A=render_mode_A,
            builder_B=builder_B, render_mode_B=render_mode_B,
            source_json_file=dict_path,
            source_skp_name=skp_name, tilenum=None))

        tiled_images_A = self.get_default(
            dictA_results, [],
            skp_name, gm_json.JSONKEY_SOURCE_TILEDIMAGES)
        tiled_images_B = self.get_default(
            dictB_results, [],
            skp_name, gm_json.JSONKEY_SOURCE_TILEDIMAGES)
        if tiled_images_A or tiled_images_B:
          num_tiles_A = len(tiled_images_A)
          num_tiles_B = len(tiled_images_B)
          num_tiles = max(num_tiles_A, num_tiles_B)
          for tile_num in range(num_tiles):
            imagepairs_for_this_skp.append(self._create_image_pair(
                image_dict_A=(tiled_images_A[tile_num]
                              if tile_num < num_tiles_A else None),
                image_dict_B=(tiled_images_B[tile_num]
                              if tile_num < num_tiles_B else None),
                image_A_base_url=image_A_base_url,
                image_B_base_url=image_B_base_url,
                builder_A=builder_A, render_mode_A=render_mode_A,
                builder_B=builder_B, render_mode_B=render_mode_B,
                source_json_file=dict_path,
                source_skp_name=skp_name, tilenum=tile_num))

        for one_imagepair in imagepairs_for_this_skp:
          if one_imagepair:
            all_image_pairs.add_image_pair(one_imagepair)
            result_type = one_imagepair.extra_columns_dict\
                [COLUMN__RESULT_TYPE]
            if result_type != results.KEY__RESULT_TYPE__SUCCEEDED:
              failing_image_pairs.add_image_pair(one_imagepair)

    logging.info('Finished adding imagepairs to queue.')
    self._image_diff_db.log_queue_size_if_changed(limit_verbosity=False)

    if self._prefetch_only:
      return None
    else:
      return {
          results.KEY__HEADER__RESULTS_ALL: all_image_pairs.as_dict(
              column_ids_in_order=ORDERED_COLUMN_IDS),
          results.KEY__HEADER__RESULTS_FAILURES: failing_image_pairs.as_dict(
              column_ids_in_order=ORDERED_COLUMN_IDS),
      }

  def _validate_dict_version(self, result_dict):
    """Raises Exception if the dict is not the type/version we know how to read.

    Args:
      result_dict: dictionary holding output of render_pictures; if None,
          this method will return without raising an Exception
    """
    # TODO(stephana): These values should be defined as constants somewhere,
    # to be kept in sync between this file and writable_expectations.py
    expected_header_type = 'ChecksummedImages'
    expected_header_revision = 1

    if result_dict == None:
      return
    header = result_dict[gm_json.JSONKEY_HEADER]
    header_type = header[gm_json.JSONKEY_HEADER_TYPE]
    if header_type != expected_header_type:
      raise Exception('expected header_type "%s", but got "%s"' % (
          expected_header_type, header_type))
    header_revision = header[gm_json.JSONKEY_HEADER_REVISION]
    if header_revision != expected_header_revision:
      raise Exception('expected header_revision %d, but got %d' % (
          expected_header_revision, header_revision))

  def _create_image_pair(self, image_dict_A, image_dict_B,
                         image_A_base_url, image_B_base_url,
                         builder_A, render_mode_A,
                         builder_B, render_mode_B,
                         source_json_file,
                         source_skp_name, tilenum):
    """Creates an ImagePair object for this pair of images.

    Args:
      image_dict_A: dict with JSONKEY_IMAGE_* keys, or None if no image
      image_dict_B: dict with JSONKEY_IMAGE_* keys, or None if no image
      image_A_base_url: base URL for image A
      image_B_base_url: base URL for image B
      builder_A: builder that created image set A or None if unknow
      render_mode_A: render mode used to generate image set A or None if
                     unknown.
      builder_B: builder that created image set A or None if unknow
      render_mode_B: render mode used to generate image set A or None if
                     unknown.
      source_json_file: string; relative path of the JSON file where this
                        result came from, within setA and setB.
      source_skp_name: string; name of the source SKP file
      tilenum: which tile, or None if a wholeimage

    Returns:
      An ImagePair object, or None if both image_dict_A and image_dict_B are
      None.
    """
    if (not image_dict_A) and (not image_dict_B):
      return None

    def _checksum_and_relative_url(dic):
      if dic:
        return ((dic[gm_json.JSONKEY_IMAGE_CHECKSUMALGORITHM],
                 int(dic[gm_json.JSONKEY_IMAGE_CHECKSUMVALUE])),
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
        COLUMN__RESULT_TYPE: result_type,
        COLUMN__SOURCE_SKP: source_skp_name,
        COLUMN__BUILDER_A: builder_A,
        COLUMN__RENDER_MODE_A: render_mode_A,
        COLUMN__BUILDER_B: builder_B,
        COLUMN__RENDER_MODE_B: render_mode_B,
    }
    if tilenum == None:
      extra_columns_dict[COLUMN__TILED_OR_WHOLE] = COLUMN__TILED_OR_WHOLE__WHOLE
      extra_columns_dict[COLUMN__TILENUM] = 'N/A'
    else:
      extra_columns_dict[COLUMN__TILED_OR_WHOLE] = COLUMN__TILED_OR_WHOLE__TILED
      extra_columns_dict[COLUMN__TILENUM] = str(tilenum)

    try:
      return imagepair.ImagePair(
          image_diff_db=self._image_diff_db,
          imageA_base_url=image_A_base_url,
          imageB_base_url=image_B_base_url,
          imageA_relative_url=imageA_relative_url,
          imageB_relative_url=imageB_relative_url,
          extra_columns=extra_columns_dict,
          source_json_file=source_json_file,
          download_all_images=self._download_all_images)
    except (KeyError, TypeError):
      logging.exception(
          'got exception while creating ImagePair for'
          ' urlPair=("%s","%s"), source_skp_name="%s", tilenum="%s"' % (
              imageA_relative_url, imageB_relative_url, source_skp_name,
              tilenum))
      return None

  def _copy_dir_contents(self, source_dir, dest_dir):
    """Copy all contents of source_dir into dest_dir, recursing into subdirs.

    Args:
      source_dir: path to source dir (GS URL, local filepath, or a special
          "repo:" URL type that points at a file within our Skia checkout)
      dest_dir: path to destination dir (local filepath)

    The copy operates as a "merge with overwrite": any files in source_dir will
    be "overlaid" on top of the existing content in dest_dir.  Existing files
    with the same names will be overwritten.
    """
    if gs_utils.GSUtils.is_gs_url(source_dir):
      (bucket, path) = gs_utils.GSUtils.split_gs_url(source_dir)
      self._gs.download_dir_contents(source_bucket=bucket, source_dir=path,
                                     dest_dir=dest_dir)
    elif source_dir.lower().startswith(REPO_URL_PREFIX):
      repo_dir = os.path.join(REPO_BASEPATH, source_dir[len(REPO_URL_PREFIX):])
      shutil.copytree(repo_dir, dest_dir)
    else:
      shutil.copytree(source_dir, dest_dir)

  def _get_repo_revision(self, source_dir):
    """Get the commit hash of source_dir, IF it refers to a git checkout.

    Args:
      source_dir: path to source dir (GS URL, local filepath, or a special
          "repo:" URL type that points at a file within our Skia checkout;
          only the "repo:" URL type will have a commit hash.
    """
    if source_dir.lower().startswith(REPO_URL_PREFIX):
      repo_dir = os.path.join(REPO_BASEPATH, source_dir[len(REPO_URL_PREFIX):])
      return subprocess.check_output(
          args=[git_utils.GIT, 'rev-parse', 'HEAD'], cwd=repo_dir).strip()
    else:
      return None
