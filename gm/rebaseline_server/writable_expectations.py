#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Expectations on local disk that we can modify.
"""

# System-level imports
import logging
import os
import re

# Must fix up PYTHONPATH before importing from within Skia
import rs_fixpypath  # pylint: disable=W0611

# Imports from within Skia
from py.utils import git_utils
import compare_rendered_pictures
import gm_json
import imagepair
import results

FILEPATH_RE = re.compile('.+/' + gm_json.IMAGE_FILENAME_PATTERN)

SKIA_REPO = os.path.abspath(os.path.join(
    os.path.dirname(__file__), os.pardir, os.pardir, '.git'))


class WritableExpectations(git_utils.NewGitCheckout):
  """Expectations on local disk that we can modify."""

  def __init__(self, set_descriptions):
    """Creates a sandbox on local disk containing writable expectations.

    You must use the 'with' statement to create this object in such a way that
    it cleans up after itself:

    with WritableExpectations(*args) as writable_expectations:
      # make modifications
      # use the modified results
    # the sandbox on local disk is automatically cleaned up here

    Args:
      set_descriptions: SET_DESCRIPTIONS dict describing the set we want to
          update expectations within; this tells us the subdirectory within the
          Skia repo where we keep these expectations, and the commithash at
          which the user evaluated new baselines.
    """
    file_section = set_descriptions[results.KEY__SET_DESCRIPTIONS__SECTION]
    assert file_section == gm_json.JSONKEY_EXPECTEDRESULTS

    source_dir = _unicode_to_ascii(
        set_descriptions[results.KEY__SET_DESCRIPTIONS__DIR])
    assert source_dir.startswith(compare_rendered_pictures.REPO_URL_PREFIX)
    repo_subdir = source_dir[len(compare_rendered_pictures.REPO_URL_PREFIX):]
    repo_revision = _unicode_to_ascii(
        set_descriptions[results.KEY__SET_DESCRIPTIONS__REPO_REVISION])

    logging.info('Creating a writable Skia checkout at revision "%s"...' %
                 repo_revision)
    super(WritableExpectations, self).__init__(
        repository=SKIA_REPO, commit=repo_revision, subdir=repo_subdir)

  def modify(self, modifications):
    """Modify the contents of the checkout, using modifications from the UI.

    Args:
      modifications: data[KEY__LIVE_EDITS__MODIFICATIONS] coming back from the
          rebaseline_server UI frontend
    """
    logging.info('Reading in dicts from writable Skia checkout in %s ...' %
                 self.root)
    dicts = results.BaseComparisons.read_dicts_from_root(self.root)

    # Make sure we have expected-results sections in all our output dicts.
    for pathname, adict in dicts.iteritems():
      if not adict:
        adict = {
          # TODO(stephana): These values should be defined as constants
          # somewhere, to be kept in sync between this file and
          # compare_rendered_pictures.py.
          gm_json.JSONKEY_HEADER: {
            gm_json.JSONKEY_HEADER_TYPE: 'ChecksummedImages',
            gm_json.JSONKEY_HEADER_REVISION: 1,
          }
        }
      if not adict.get(gm_json.JSONKEY_EXPECTEDRESULTS, None):
        adict[gm_json.JSONKEY_EXPECTEDRESULTS] = {}
      dicts[pathname] = adict

    for modification in modifications:
      expectations = modification[imagepair.KEY__IMAGEPAIRS__EXPECTATIONS]
      _add_image_info_to_expectations(
          expectations=expectations,
          filepath=modification[imagepair.KEY__IMAGEPAIRS__IMAGE_B_URL])
      extra_columns = modification[imagepair.KEY__IMAGEPAIRS__EXTRACOLUMNS]
      dictname = modification[imagepair.KEY__IMAGEPAIRS__SOURCE_JSON_FILE]
      dict_to_modify = dicts[dictname][gm_json.JSONKEY_EXPECTEDRESULTS]
      test_name = extra_columns[compare_rendered_pictures.COLUMN__SOURCE_SKP]
      test_record = dict_to_modify.get(test_name, {})
      if (extra_columns[compare_rendered_pictures.COLUMN__TILED_OR_WHOLE] ==
          compare_rendered_pictures.COLUMN__TILED_OR_WHOLE__TILED):
        test_tiles_list = test_record.get(
            gm_json.JSONKEY_SOURCE_TILEDIMAGES, [])
        tilenum = int(extra_columns[compare_rendered_pictures.COLUMN__TILENUM])
        _replace_list_item(test_tiles_list, tilenum, expectations)
        test_record[gm_json.JSONKEY_SOURCE_TILEDIMAGES] = test_tiles_list
      else:
        test_record[gm_json.JSONKEY_SOURCE_WHOLEIMAGE] = expectations
      dict_to_modify[test_name] = test_record

    # Write the modified files back to disk.
    self._write_dicts_to_root(meta_dict=dicts, root=self.root)

  def get_diffs(self):
    """Return patchfile describing any modifications to this checkout."""
    return self._run_in_git_root(args=[git_utils.GIT, 'diff'])

  @staticmethod
  def _write_dicts_to_root(meta_dict, root):
    """Write out multiple dictionaries in JSON format.

    Args:
      meta_dict: a builder-keyed meta-dictionary containing all the JSON
                 dictionaries we want to write out
      root: path to root of directory tree within which to write files
    """
    if not os.path.isdir(root):
      raise IOError('no directory found at path %s' % root)

    for rel_path in meta_dict.keys():
      full_path = os.path.join(root, rel_path)
      gm_json.WriteToFile(meta_dict[rel_path], full_path)


def _unicode_to_ascii(unicode_string):
  """Returns the plain ASCII form of a unicode string.

  TODO(stephana): We created this because we get unicode strings out of the
  JSON file, while the git filenames and revision tags are plain ASCII.
  There may be a better way to handle this... maybe set the JSON util to just
  return ASCII strings?
  """
  return unicode_string.encode('ascii', 'ignore')


def _replace_list_item(a_list, index, value):
  """Replaces value at index "index" within a_list.

  Args:
    a_list: a list
    index: index indicating which item in a_list to replace
    value: value to set a_list[index] to

  If a_list does not contain this index, it will be extended with None entries
  to that length.
  """
  length = len(a_list)
  while index >= length:
    a_list.append(None)
    length += 1
  a_list[index] = value


def _add_image_info_to_expectations(expectations, filepath):
  """Add JSONKEY_IMAGE_* info to an existing expectations dictionary.

  TODO(stephana): This assumes that the checksumAlgorithm and checksumValue
  can be derived from the filepath, which is currently true but may not always
  be true.

  Args:
    expectations: the expectations dict to augment
    filepath: relative path to the image file
  """
  (checksum_algorithm, checksum_value) = FILEPATH_RE.match(filepath).groups()
  expectations[gm_json.JSONKEY_IMAGE_CHECKSUMALGORITHM] = checksum_algorithm
  expectations[gm_json.JSONKEY_IMAGE_CHECKSUMVALUE] = checksum_value
  expectations[gm_json.JSONKEY_IMAGE_FILEPATH] = filepath
