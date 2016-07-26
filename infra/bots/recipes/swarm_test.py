# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming test.


DEPS = [
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/raw_io',
  'skia',
]


TEST_BUILDERS = {
  'client.skia': {
    'skiabot-linux-swarm-000': [
      'Test-Android-GCC-GalaxyS3-GPU-Mali400-Arm7-Debug',
      'Test-Android-GCC-Nexus7-GPU-Tegra3-Arm7-Debug',
      'Test-iOS-Clang-iPad4-GPU-SGX554-Arm7-Debug',
      'Test-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Coverage-Trybot',
      'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug',
      'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-MSAN',
      'Test-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-Valgrind',
      'Test-Win8-MSVC-ShuttleB-CPU-AVX2-x86_64-Release-Trybot',
    ],
  },
}


def RunSteps(api):
  api.skia.setup()
  api.skia.test_steps()
  api.skia.cleanup_steps()
  api.skia.check_failure()


def GenTests(api):
  def AndroidTestData(builder, adb=None):
    test_data = (
        api.step_data(
            'get EXTERNAL_STORAGE dir',
            stdout=api.raw_io.output('/storage/emulated/legacy')) +
        api.step_data(
            'read SKP_VERSION',
            stdout=api.raw_io.output('42')) +
        api.step_data(
            'read SK_IMAGE_VERSION',
            stdout=api.raw_io.output('42')) +
       api.step_data(
            'exists skia_dm',
            stdout=api.raw_io.output(''))
    )
    if 'GalaxyS3' not in builder:
      test_data += api.step_data(
          'adb root',
          stdout=api.raw_io.output('restarting adbd as root'))
    if adb:
      test_data += api.step_data(
          'which adb',
          stdout=api.raw_io.output(adb))
    else:
      test_data += api.step_data(
        'which adb',
        retcode=1)

    return test_data

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
              api.path['slave_build'].join('skia'),
              api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                           'skimage', 'VERSION'),
              api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                           'skp', 'VERSION'),
              api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
          )
        )
        if ('Android' in builder and
            ('Test' in builder or 'Perf' in builder) and
            not 'Appurify' in builder):
          test += AndroidTestData(builder)
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
        api.path['slave_build'].join('skia'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    api.step_data('dm', retcode=1)
  )

  builder = 'Test-Android-GCC-Nexus7-GPU-Tegra3-Arm7-Debug'
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
        api.path['slave_build'].join('skia'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    AndroidTestData(builder) +
    api.step_data('read SKP_VERSION',
                  stdout=api.raw_io.output('42')) +
    api.step_data('read SK_IMAGE_VERSION',
                  stdout=api.raw_io.output('42')) +
    api.step_data('get uninteresting hashes', retcode=1)
  )

  yield (
    api.test('download_and_push_skps') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
    api.path.exists(
        api.path['slave_build'].join('skia'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    AndroidTestData(builder) +
    api.step_data('read SKP_VERSION',
                  stdout=api.raw_io.output('2')) +
    api.step_data('read SK_IMAGE_VERSION',
                  stdout=api.raw_io.output('42')) +
    api.step_data(
        'exists skps',
        stdout=api.raw_io.output(''))
  )

  yield (
    api.test('missing_SKP_VERSION_device') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
    api.path.exists(
        api.path['slave_build'].join('skia'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    AndroidTestData(builder) +
    api.step_data('read SKP_VERSION',
                  retcode=1) +
    api.step_data('read SK_IMAGE_VERSION',
                  stdout=api.raw_io.output('42')) +
    api.step_data(
        'exists skps',
        stdout=api.raw_io.output(''))
  )

  yield (
    api.test('download_and_push_skimage') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
    api.path.exists(
        api.path['slave_build'].join('skia'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    AndroidTestData(builder) +
    api.step_data('read SKP_VERSION',
                  stdout=api.raw_io.output('42')) +
    api.step_data('read SK_IMAGE_VERSION',
                  stdout=api.raw_io.output('2')) +
    api.step_data(
        'exists skia_images',
        stdout=api.raw_io.output(''))
  )

  yield (
    api.test('missing_SK_IMAGE_VERSION_device') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
    api.path.exists(
        api.path['slave_build'].join('skia'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    AndroidTestData(builder) +
    api.step_data('read SKP_VERSION',
                  stdout=api.raw_io.output('42')) +
    api.step_data('read SK_IMAGE_VERSION',
                  retcode=1) +
    api.step_data(
        'exists skia_images',
        stdout=api.raw_io.output(''))
  )

  yield (
    api.test('adb_in_path') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
    api.path.exists(
        api.path['slave_build'].join('skia'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    AndroidTestData(builder, adb='/usr/bin/adb') +
    api.step_data('read SKP_VERSION',
                  stdout=api.raw_io.output('42')) +
    api.step_data('read SK_IMAGE_VERSION',
                  stdout=api.raw_io.output('42'))
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
        api.path['slave_build'].join('skia'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    api.platform('win', 64)
  )

  builder = 'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug'
  yield (
    api.test('legacy_skimage_version') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
    api.path.exists(
        api.path['slave_build'].join('skia'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
    )
  )

  yield (
    api.test('legacy_skp_version') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
    api.path.exists(
        api.path['slave_build'].join('skia'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
    )
  )
