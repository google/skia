# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'recipe_engine/properties',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  info = [
    api.vars.upload_dm_results,
    api.vars.upload_perf_results,
    api.vars.swarming_bot_id,
    api.vars.swarming_task_id,
  ]
  if api.vars.is_linux:
    assert len(info) == 4  # Make pylint happy.


TEST_BUILDERS = [
  'Build-Mac-Clang-x86_64-Debug-CommandBuffer',
  'Build-Ubuntu-GCC-x86_64-Release-Flutter_Android',
  'Build-Ubuntu-GCC-x86_64-Release-PDFium',
  'Build-Win-MSVC-x86_64-Release-Vulkan',
  'Housekeeper-Weekly-RecreateSKPs',
  'Perf-Chromecast-GCC-Chorizo-CPU-Cortex_A7-arm-Debug',
  'Perf-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Release-ASAN',
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

  buildername = 'Test-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Debug'
  yield (
      api.test('win_test') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )
  )
