# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the Skia gold_upload tests.

PYTHON_VERSION_COMPATIBILITY = "PY2+3"

DEPS = [
  'gold_upload',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
  'flavor',
  'run',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  api.flavor.setup('dm')
  api.gold_upload.upload()

def GenTests(api):
  yield (
      api.test('upload_tests') +
      api.properties(buildername='Test-Android-Clang-Pixel2XL-Some-GPU-arm64-Debug-All',
                     repository='https://skia.googlesource.com/skia.git',
                     gs_bucket='skia-infra-gm',
                     patch_ref='89/456789/12',
                     patch_set=7,
                     patch_issue=1234,
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
