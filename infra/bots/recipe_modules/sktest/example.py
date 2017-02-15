# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Example recipe w/ coverage.


DEPS = [
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/raw_io',
  'sktest',
]


TEST_BUILDERS = {
  'client.skia': {
    'skiabot-linux-swarm-000': [
      'Test-Android-Clang-AndroidOne-CPU-MT6582-arm-Release-GN_Android',
      'Test-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-GN_Android',
      'Test-Android-Clang-GalaxyS7-GPU-Adreno530-arm64-Debug-GN_Android',
      'Test-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Debug-GN_Android',
      'Test-Android-Clang-Nexus10-GPU-MaliT604-arm-Release-GN_Android',
      'Test-Android-Clang-Nexus5-GPU-Adreno330-arm-Release-Android',
      'Test-Android-Clang-Nexus6-GPU-Adreno420-arm-Debug-GN_Android',
      'Test-Android-Clang-Nexus6p-GPU-Adreno430-arm64-Debug-GN_Android_Vulkan',
      'Test-Android-Clang-Nexus7-GPU-Tegra3-arm-Debug-GN_Android',
      'Test-Android-Clang-NexusPlayer-CPU-SSE4-x86-Release-GN_Android',
      ('Test-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Release-'
       'GN_Android_Vulkan'),
      'Test-Android-Clang-PixelC-GPU-TegraX1-arm64-Debug-GN_Android',
      'Test-Mac-Clang-MacMini4.1-GPU-GeForce320M-x86_64-Debug',
      'Test-Mac-Clang-MacMini6.2-CPU-AVX-x86_64-Debug',
      'Test-Mac-Clang-MacMini6.2-GPU-HD4000-x86_64-Debug-CommandBuffer',
      'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86-Debug',
      'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug',
      'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-ASAN',
      'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-MSAN',
      'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-Shared',
      'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-TSAN',
      'Test-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-Valgrind',
      'Test-Ubuntu16-Clang-NUC-GPU-IntelIris540-x86_64-Debug-Vulkan',
      'Test-Win10-MSVC-NUC-GPU-IntelIris540-x86_64-Debug-ANGLE',
      'Test-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Debug-Vulkan',
      'Test-Win10-MSVC-ZBOX-GPU-GTX1070-x86_64-Debug-Vulkan',
      'Test-Win8-MSVC-ShuttleB-CPU-AVX2-x86_64-Release-Trybot',
      'Test-Win8-MSVC-ShuttleB-GPU-GTX960-x86_64-Debug-ANGLE',
      'Test-iOS-Clang-iPadMini4-GPU-GX6450-arm-Release',
    ],
  },
}


def RunSteps(api):
  api.sktest.run()


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
              api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                           'svg', 'VERSION'),
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

  builder = 'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug'
  yield (
    api.test('failed_dm') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
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
    api.step_data('dm', retcode=1)
  )

  builder = 'Test-Android-Clang-Nexus7-GPU-Tegra3-arm-Debug-GN_Android'
  yield (
    api.test('failed_get_hashes') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
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
    api.step_data('get uninteresting hashes', retcode=1)
  )

  builder = 'Test-Win8-MSVC-ShuttleB-CPU-AVX2-x86_64-Release-Trybot'
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

  builder = 'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86-Debug-Trybot'
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

  yield (
      api.test('nobuildbot') +
      api.properties(
          buildername=builder,
          mastername='client.skia',
          slavename='skiabot-linux-swarm-000',
          buildnumber=5,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          revision='abc123',
          nobuildbot='True',
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=builder,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      ) +
      api.step_data('get swarming bot id',
          stdout=api.raw_io.output('skia-bot-123')) +
      api.step_data('get swarming task id', stdout=api.raw_io.output('123456'))
  )

  builder = 'Test-Android-Clang-NexusPlayer-CPU-SSE4-x86-Debug-GN_Android'
  yield (
    api.test('failed_push') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
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
    api.step_data('push [START_DIR]/skia/resources/* '+
                  '/sdcard/revenge_of_the_skiabot/resources', retcode=1)
  )

  builder = 'Test-Android-Clang-Nexus10-GPU-MaliT604-arm-Debug-Android'
  yield (
    api.test('failed_pull') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
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
    api.step_data('dm', retcode=1) +
    api.step_data('pull /sdcard/revenge_of_the_skiabot/dm_out '+
                  '[CUSTOM_[SWARM_OUT_DIR]]/dm', retcode=1)
  )
