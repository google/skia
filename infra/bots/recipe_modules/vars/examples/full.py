# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/step',
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
  s = api.step('show', cmd=None)
  for p in [
      'build_dir',
      'builder_cfg',
      'builder_name',
      'cache_dir',
      'default_env',
      'extra_tokens',
      'internal_hardware_label',
      'is_internal_bot',
      'is_linux',
      'is_trybot',
      'issue',
      'patch_storage',
      'patchset',
      'role',
      'workdir',
      'swarming_out_dir',
      'tmp_dir',
      ]:
    s.presentation.properties[p] = str(getattr(api.vars, p))


TEST_BUILDERS = [
  'Build-Debian10-Clang-x86_64-Release-SKNX_NO_SIMD',
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

  buildername = 'Upload-Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All-ASAN_Vulkan'
  yield (
      api.test('integer_issue') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     patch_issue='0',
                     patch_set='0')
  )
