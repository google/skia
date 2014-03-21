#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Repackage expected/actual GM results as needed by our HTML rebaseline viewer.
"""

# System-level imports
import os
import re
import sys

# Imports from within Skia
#
# We need to add the 'gm' directory, so that we can import gm_json.py within
# that directory.  That script allows us to parse the actual-results.json file
# written out by the GM tool.
# Make sure that the 'gm' dir is in the PYTHONPATH, but add it at the *end*
# so any dirs that are already in the PYTHONPATH will be preferred.
PARENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
GM_DIRECTORY = os.path.dirname(PARENT_DIRECTORY)
if GM_DIRECTORY not in sys.path:
  sys.path.append(GM_DIRECTORY)
import gm_json

# Keys used to link an image to a particular GM test.
# NOTE: Keep these in sync with static/constants.js
REBASELINE_SERVER_SCHEMA_VERSION_NUMBER = 2
KEY__EXPECTATIONS__BUGS = gm_json.JSONKEY_EXPECTEDRESULTS_BUGS
KEY__EXPECTATIONS__IGNOREFAILURE = gm_json.JSONKEY_EXPECTEDRESULTS_IGNOREFAILURE
KEY__EXPECTATIONS__REVIEWED = gm_json.JSONKEY_EXPECTEDRESULTS_REVIEWED
KEY__EXTRACOLUMN__BUILDER = 'builder'
KEY__EXTRACOLUMN__CONFIG = 'config'
KEY__EXTRACOLUMN__RESULT_TYPE = 'resultType'
KEY__EXTRACOLUMN__TEST = 'test'
KEY__HEADER = 'header'
KEY__HEADER__DATAHASH = 'dataHash'
KEY__HEADER__IS_EDITABLE = 'isEditable'
KEY__HEADER__IS_EXPORTED = 'isExported'
KEY__HEADER__IS_STILL_LOADING = 'resultsStillLoading'
KEY__HEADER__RESULTS_ALL = 'all'
KEY__HEADER__RESULTS_FAILURES = 'failures'
KEY__HEADER__SCHEMA_VERSION = 'schemaVersion'
KEY__HEADER__TIME_NEXT_UPDATE_AVAILABLE = 'timeNextUpdateAvailable'
KEY__HEADER__TIME_UPDATED = 'timeUpdated'
KEY__HEADER__TYPE = 'type'
KEY__NEW_IMAGE_URL = 'newImageUrl'
KEY__RESULT_TYPE__FAILED = gm_json.JSONKEY_ACTUALRESULTS_FAILED
KEY__RESULT_TYPE__FAILUREIGNORED = gm_json.JSONKEY_ACTUALRESULTS_FAILUREIGNORED
KEY__RESULT_TYPE__NOCOMPARISON = gm_json.JSONKEY_ACTUALRESULTS_NOCOMPARISON
KEY__RESULT_TYPE__SUCCEEDED = gm_json.JSONKEY_ACTUALRESULTS_SUCCEEDED

IMAGE_FILENAME_RE = re.compile(gm_json.IMAGE_FILENAME_PATTERN)
IMAGE_FILENAME_FORMATTER = '%s_%s.png'  # pass in (testname, config)
