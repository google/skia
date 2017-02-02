# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Example recipe w/ coverage.


DEPS = [
  'perf',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/raw_io',
]


TEST_BUILDERS = {
  'client.skia': {
    'skiabot-linux-swarm-000': [
      ('Perf-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Debug' +
       '-GN_Android_Vulkan'),
      'Perf-Android-Clang-Nexus5-GPU-Adreno330-arm-Debug-GN_Android',
      'Perf-Android-Clang-Nexus6-GPU-Adreno420-arm-Release-GN_Android',
      'Perf-Android-Clang-Nexus7-GPU-Tegra3-arm-Release-GN_Android',
      'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Release-GN_Android',
      ('Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Release-GN_'
       'Android_Vulkan'),
      'Perf-Android-Clang-PixelC-GPU-TegraX1-arm64-Release-GN_Android',
      'Perf-Mac-Clang-MacMini6.2-CPU-AVX-x86_64-Release-GN',
      'Perf-Mac-Clang-MacMini6.2-GPU-HD4000-x86_64-Debug-CommandBuffer',
      'Perf-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Release-GN',
      'Perf-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-Valgrind',
      'Perf-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-ANGLE',
      'Perf-Win-MSVC-GCE-CPU-AVX2-x86_64-Debug',
      'Perf-Win-MSVC-GCE-CPU-AVX2-x86_64-Release',
      'Perf-Win10-MSVC-NUC-GPU-IntelIris540-x86_64-Release-ANGLE',
      'Perf-Win8-MSVC-ShuttleB-GPU-GTX960-x86_64-Debug-ANGLE',
      'Perf-Win8-MSVC-ShuttleB-GPU-HD4600-x86_64-Release-Trybot',
      'Perf-iOS-Clang-iPad4-GPU-SGX554-Arm7-Debug',
    ],
  },
}


def RunSteps(api):
  api.perf.run()


def GenTests(api):
  for mastername, slaves in TEST_BUILDERS.iteritems():
    for slavename, builders_by_slave in slaves.iteritems():
      for builder in builders_by_slave:
        test = (
          api.test(builder) +
          api.properties(buildername=builder,
                         mastername=mastername,
                         slavename=slavename,
                         buildnumber=5,
                         revision='abc123',
                         path_config='kitchen',
                         swarm_out_dir='[SWARM_OUT_DIR]') +
          api.path.exists(
              api.path['start_dir'].join('skia'),
              api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                           'skimage', 'VERSION'),
              api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                           'skp', 'VERSION'),
              api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
          )
        )
        if 'Trybot' in builder:
          test += api.properties(issue=500,
                                 patchset=1,
                                 rietveld='https://codereview.chromium.org')
        if 'Win' in builder:
          test += api.platform('win', 64)

        yield test

  builder = 'Perf-Win8-MSVC-ShuttleB-GPU-HD4600-x86_64-Release-Trybot'
  yield (
    api.test('big_issue_number') +
    api.properties(buildername=builder,
                   mastername='client.skia.compile',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=5,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   rietveld='https://codereview.chromium.org',
                   patchset=1,
                   issue=2147533002L) +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    api.platform('win', 64)
  )

  builder = ('Perf-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-Valgrind-'
             'Trybot')
  yield (
      api.test('recipe_with_gerrit_patch') +
      api.properties(
          buildername=builder,
          mastername='client.skia',
          slavename='skiabot-linux-swarm-000',
          buildnumber=5,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          revision='abc123',
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=builder,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )
  )

  builder = 'Perf-Win8-MSVC-ShuttleB-GPU-HD4600-x86_64-Release-Trybot'
  yield (
      api.test('nobuildbot') +
      api.properties(
          buildername=builder,
          mastername='client.skia',
          slavename='skiabot-linux-swarm-000',
          buildnumber=5,
          revision='abc123',
          path_config='kitchen',
          nobuildbot='True',
          swarm_out_dir='[SWARM_OUT_DIR]',
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=builder,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      ) +
      api.path.exists(
          api.path['start_dir'].join('skia'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                       'skimage', 'VERSION'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                       'skp', 'VERSION'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                       'svg', 'VERSION'),
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      ) +
      api.platform('win', 64) +
      api.step_data('get swarming bot id',
          stdout=api.raw_io.output('skia-bot-123')) +
      api.step_data('get swarming task id', stdout=api.raw_io.output('123456'))
  )
