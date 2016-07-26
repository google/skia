#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" Upload benchmark performance data results. """

import gzip
import os
import os.path
import re
import subprocess
import sys
import tempfile

from common.skia import builder_name_schema
from common.skia import global_constants
from datetime import datetime


def _UploadJSONResults(builder_name, build_number, dest_gsbase, gs_subdir,
                       full_json_path, gzipped=True, gsutil_path='gsutil',
                       issue_number=None):
  now = datetime.utcnow()
  gs_json_path = '/'.join((str(now.year).zfill(4), str(now.month).zfill(2),
                           str(now.day).zfill(2), str(now.hour).zfill(2)))
  gs_dir = '/'.join((gs_subdir, gs_json_path, builder_name))
  if builder_name_schema.IsTrybot(builder_name):
    if not issue_number:
      raise Exception('issue_number build property is missing!')
    gs_dir = '/'.join(('trybot', gs_dir, build_number, issue_number))
  full_path_to_upload = full_json_path
  file_to_upload = os.path.basename(full_path_to_upload)
  http_header = ['Content-Type:application/json']
  if gzipped:
    http_header.append('Content-Encoding:gzip')
    gzipped_file = os.path.join(tempfile.gettempdir(), file_to_upload)
    # Apply gzip.
    with open(full_path_to_upload, 'rb') as f_in:
      with gzip.open(gzipped_file, 'wb') as f_out:
        f_out.writelines(f_in)
    full_path_to_upload = gzipped_file
  cmd = ['python', gsutil_path]
  for header in http_header:
    cmd.extend(['-h', header])
  cmd.extend(['cp', '-a', 'public-read', full_path_to_upload,
              '/'.join((dest_gsbase, gs_dir, file_to_upload))])
  print ' '.join(cmd)
  subprocess.check_call(cmd)


def main(builder_name, build_number, perf_data_dir, got_revision, gsutil_path,
         issue_number=None):
  """Uploads gzipped nanobench JSON data."""
  # Find the nanobench JSON
  file_list = os.listdir(perf_data_dir)
  RE_FILE_SEARCH = re.compile(
      'nanobench_({})_[0-9]+\.json'.format(got_revision))
  nanobench_name = None

  for file_name in file_list:
    if RE_FILE_SEARCH.search(file_name):
      nanobench_name = file_name
      break

  if nanobench_name:
    dest_gsbase = 'gs://' + global_constants.GS_GM_BUCKET
    nanobench_json_file = os.path.join(perf_data_dir,
                                       nanobench_name)
    _UploadJSONResults(builder_name, build_number, dest_gsbase, 'nano-json-v1',
                       nanobench_json_file, gsutil_path=gsutil_path,
                       issue_number=issue_number)


if __name__ == '__main__':
  main(*sys.argv[1:])

