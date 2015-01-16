#!/usr/bin/env python
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Schema of the JSON summary file written out by the GM tool.

This must be kept in sync with the kJsonKey_ constants in gm_expectations.cpp !
"""

__author__ = 'Elliot Poger'


# system-level imports
import io
import json
import os
import posixpath
import re


# Key strings used in GM results JSON files (both expected-results.json and
# actual-results.json).
#
# NOTE: These constants must be kept in sync with the kJsonKey_ constants in
# gm_expectations.cpp and tools/PictureRenderer.cpp !
# Eric suggests: create gm/gm_expectations_constants.h containing ONLY variable
# declarations so as to be readable by both gm/gm_expectations.cpp and Python.


JSONKEY_ACTUALRESULTS = 'actual-results'

# Tests whose results failed to match expectations.
JSONKEY_ACTUALRESULTS_FAILED = 'failed'

# Tests whose results failed to match expectations, but IGNOREFAILURE causes
# us to take them less seriously.
JSONKEY_ACTUALRESULTS_FAILUREIGNORED = 'failure-ignored'

# Tests for which we do not have any expectations.  They may be new tests that
# we haven't had a chance to check in expectations for yet, or we may have
# consciously decided to leave them without expectations because we are unhappy
# with the results (although we should try to move away from that, and instead
# check in expectations with the IGNOREFAILURE flag set).
JSONKEY_ACTUALRESULTS_NOCOMPARISON = 'no-comparison'

# Tests whose results matched their expectations.
JSONKEY_ACTUALRESULTS_SUCCEEDED = 'succeeded'


# Descriptions of the result set as a whole.
JSONKEY_DESCRIPTIONS = 'descriptions'
JSONKEY_DESCRIPTIONS_BUILDER = 'builder'
JSONKEY_DESCRIPTIONS_RENDER_MODE = 'renderMode'

JSONKEY_EXPECTEDRESULTS = 'expected-results'

# One or more [HashType/DigestValue] pairs representing valid results for this
# test.  Typically, there will just be one pair, but we allow for multiple
# expectations, and the test will pass if any one of them is matched.
JSONKEY_EXPECTEDRESULTS_ALLOWEDDIGESTS = 'allowed-digests'

# Optional: one or more integers listing Skia bugs (under
# https://code.google.com/p/skia/issues/list ) that pertain to this expectation.
JSONKEY_EXPECTEDRESULTS_BUGS = 'bugs'

# If IGNOREFAILURE is set to True, a failure of this test will be reported
# within the FAILUREIGNORED section (thus NOT causing the buildbots to go red)
# rather than the FAILED section (which WOULD cause the buildbots to go red).
JSONKEY_EXPECTEDRESULTS_IGNOREFAILURE = 'ignore-failure'

# Optional: a free-form text string with human-readable information about
# this expectation.
JSONKEY_EXPECTEDRESULTS_NOTES = 'notes'

# Optional: boolean indicating whether this expectation was reviewed/approved
# by a human being.
# If True: a human looked at this image and approved it.
# If False: this expectation was committed blind.  (In such a case, please
#   add notes indicating why!)
# If absent: this expectation was committed by a tool that didn't enforce human
#   review of expectations.
JSONKEY_EXPECTEDRESULTS_REVIEWED = 'reviewed-by-human'

# Allowed hash types for test expectations.
JSONKEY_HASHTYPE_BITMAP_64BITMD5 = 'bitmap-64bitMD5'

JSONKEY_HEADER = 'header'
JSONKEY_HEADER_TYPE = 'type'
JSONKEY_HEADER_REVISION = 'revision'
JSONKEY_IMAGE_CHECKSUMALGORITHM = 'checksumAlgorithm'
JSONKEY_IMAGE_CHECKSUMVALUE = 'checksumValue'
JSONKEY_IMAGE_COMPARISONRESULT = 'comparisonResult'
JSONKEY_IMAGE_FILEPATH = 'filepath'
JSONKEY_SOURCE_TILEDIMAGES = 'tiled-images'
JSONKEY_SOURCE_WHOLEIMAGE = 'whole-image'
JSONKEY_IMAGE_BASE_GS_URL = 'image-base-gs-url'


# Root directory where the buildbots store their actually-generated images...
#  as a publicly readable HTTP URL:
GM_ACTUALS_ROOT_HTTP_URL = (
    'http://chromium-skia-gm.commondatastorage.googleapis.com/gm')
#  as a GS URL that allows credential-protected write access:
GM_ACTUALS_ROOT_GS_URL = 'gs://chromium-skia-gm/gm'

# Pattern used to assemble each image's filename
IMAGE_FILENAME_PATTERN = '(.+)_(.+)\.png'  # matches (testname, config)

# Pattern used to create image URLs, relative to some base URL.
GM_RELATIVE_URL_FORMATTER = '%s/%s/%s.png' # pass in (hash_type, test_name,
                                           #          hash_digest)
GM_RELATIVE_URL_PATTERN = '(.+)/(.+)/(.+).png' # matches (hash_type, test_name,
                                               #          hash_digest)
GM_RELATIVE_URL_RE = re.compile(GM_RELATIVE_URL_PATTERN)


def CreateGmActualUrl(test_name, hash_type, hash_digest,
                      gm_actuals_root_url=GM_ACTUALS_ROOT_HTTP_URL):
  """Return the URL we can use to download a particular version of
  the actually-generated image for this particular GM test.

  test_name: name of the test, e.g. 'perlinnoise'
  hash_type: string indicating the hash type used to generate hash_digest,
             e.g. JSONKEY_HASHTYPE_BITMAP_64BITMD5
  hash_digest: the hash digest of the image to retrieve
  gm_actuals_root_url: root url where actual images are stored
  """
  return posixpath.join(
      gm_actuals_root_url, CreateGmRelativeUrl(
          test_name=test_name, hash_type=hash_type, hash_digest=hash_digest))


def CreateGmRelativeUrl(test_name, hash_type, hash_digest):
  """Returns a relative URL pointing at a test result's image.

  Returns the URL we can use to download a particular version of
  the actually-generated image for this particular GM test,
  relative to the URL root.

  Args:
    test_name: name of the test, e.g. 'perlinnoise'
    hash_type: string indicating the hash type used to generate hash_digest,
               e.g. JSONKEY_HASHTYPE_BITMAP_64BITMD5
    hash_digest: the hash digest of the image to retrieve
  """
  return GM_RELATIVE_URL_FORMATTER % (hash_type, test_name, hash_digest)


def SplitGmRelativeUrl(url):
  """Splits the relative URL into (test_name, hash_type, hash_digest) tuple.

  This is the inverse of CreateGmRelativeUrl().

  Args:
    url: a URL generated with CreateGmRelativeUrl().

  Returns: (test_name, hash_type, hash_digest) tuple.
  """
  hash_type, test_name, hash_digest = GM_RELATIVE_URL_RE.match(url).groups()
  return (test_name, hash_type, hash_digest)


def LoadFromString(file_contents):
  """Loads the JSON summary written out by the GM tool.

     Returns a dictionary keyed by the values listed as JSONKEY_ constants
     above; if file_contents is empty, returns None."""
  # TODO(epoger): we should add a version number to the JSON file to ensure
  # that the writer and reader agree on the schema (raising an exception
  # otherwise).
  if not file_contents:
    return None
  json_dict = json.loads(file_contents)
  return json_dict


def LoadFromFile(file_path):
  """Loads the JSON summary written out by the GM tool.
     Returns a dictionary keyed by the values listed as JSONKEY_ constants
     above."""
  file_contents = open(file_path, 'r').read()
  return LoadFromString(file_contents)


def WriteToFile(json_dict, file_path):
  """Writes the JSON summary in json_dict out to file_path.

  The file is written Unix-style (each line ends with just LF, not CRLF);
  see https://code.google.com/p/skia/issues/detail?id=1815 for reasons."""
  with io.open(file_path, mode='w', newline='', encoding='utf-8') as outfile:
    outfile.write(unicode(json.dumps(json_dict, outfile, sort_keys=True,
                                     indent=2)))
