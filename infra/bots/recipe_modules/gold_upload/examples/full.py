# Copyright 2021 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the Skia gold_upload tests.


DEPS = [
  'gold_upload',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
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
      api.properties(buildername='Test-Android12-Clang-Pixel5-GPU-Adreno620-arm64-Release-All-Android_Vulkan',
                     repository='https://skia.googlesource.com/skia.git',
                     gs_bucket='skia-infra-gm',
                     patch_ref='89/456789/12',
                     patch_set=7,
                     patch_issue=1234,
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
  yield (
      api.test('upload_mac') +
      api.properties(buildername='Test-Mac12-Clang-MacBookPro16.2-GPU-IntelIrisPlus-x86_64-Debug-All-Graphite',
                     repository='https://skia.googlesource.com/skia.git',
                     gs_bucket='skia-infra-gm',
                     patch_ref='89/456789/12',
                     patch_set=7,
                     patch_issue=1234,
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.platform('mac', 64)
  )
