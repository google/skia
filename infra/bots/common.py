#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import os
import shutil
import subprocess


GS_GM_BUCKET = 'chromium-skia-gm'

GS_SUBDIR_TMPL_SK_IMAGE = 'skimage/v%s'
GS_SUBDIR_TMPL_SKP = 'playback_%s/skps'

VERSION_FILE_SK_IMAGE = 'SK_IMAGE_VERSION'
VERSION_FILE_SKP = 'SKP_VERSION'


def download_dir(skia_dir, tmp_dir, version_file, gs_path_tmpl, dst_dir):
  # Ensure that the tmp_dir exists.
  if not os.path.isdir(tmp_dir):
    os.makedirs(tmp_dir)

  # Get the expected version.
  with open(os.path.join(skia_dir, version_file)) as f:
    expected_version = f.read().rstrip()

  print 'Expected %s = %s' % (version_file, expected_version)

  # Get the actually-downloaded version, if we have one.
  actual_version_file = os.path.join(tmp_dir, version_file)
  try:
    with open(actual_version_file) as f:
      actual_version = f.read().rstrip()
  except IOError:
    actual_version = -1

  print 'Actual   %s = %s' % (version_file, actual_version)

  # If we don't have the desired version, download it.
  if actual_version != expected_version:
    if actual_version != -1:
      os.remove(actual_version_file)
    if os.path.isdir(dst_dir):
      shutil.rmtree(dst_dir)
    os.makedirs(dst_dir)
    gs_path = 'gs://%s/%s/*' % (GS_GM_BUCKET, gs_path_tmpl % expected_version)
    print 'Downloading from %s' % gs_path
    subprocess.check_call(['gsutil', 'cp', '-R', gs_path, dst_dir])
    with open(actual_version_file, 'w') as f:
      f.write(expected_version)
