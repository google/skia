# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Top-level presubmit script for Skia.

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into gcl.
"""


def CheckChangeOnUpload(input_api, output_api):
  """Presubmit checks for the change on upload."""
  return []


def CheckChangeOnCommit(input_api, output_api):
  """Presubmit checks for the change on commit.

  The following are the presubmit checks:
  * Ensures that the Skia tree is not closed in
    http://skia-tree-status.appspot.com/
  """
  results = []
  results.extend(
      input_api.canned_checks.CheckTreeIsOpen(
          input_api, output_api, json_url=(
              'http://skia-tree-status.appspot.com/banner-status?format=json')))
  return results

