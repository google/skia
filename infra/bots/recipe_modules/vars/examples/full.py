# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  info = [
    api.vars.swarming_bot_id,
    api.vars.swarming_task_id,
  ]
  if api.vars.is_linux:
    assert len(info) == 2  # Make pylint happy.


TEST_BUILDERS = [
  'Build-Debian9-Clang-x86_64-Release-SKNX_NO_SIMD',
  'Housekeeper-Weekly-RecreateSKPs',
]


def GenTests(api):
  for buildername in TEST_BUILDERS:
    yield (
        api.test(buildername) +
        api.properties(buildername=buildername,
                       repository='https://skia.googlesource.com/skia.git',
                       revision='abc123',
                       path_config='kitchen',
                       swarm_out_dir='[SWARM_OUT_DIR]')
    )

  buildername = 'Test-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Debug-All'
  yield (
      api.test('win_test') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     patch_storage='gerrit') +
      api.platform('win', 64) +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )
  )
