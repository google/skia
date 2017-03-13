# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Example recipe w/ coverage.


DEPS = [
  'compile',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
]


TEST_BUILDERS = {
  'client.skia.compile': {
    'skiabot-linux-swarm-000': [
      'Build-Mac-Clang-arm64-Debug-iOS',
      'Build-Mac-Clang-mipsel-Debug-GN_Android',
      'Build-Mac-Clang-x64-Release-iOS',
      'Build-Mac-Clang-x86_64-Debug-CommandBuffer',
      'Build-Mac-Clang-x86_64-Release-GN',
      'Build-Ubuntu-Clang-arm64-Debug-GN_Android-Trybot',
      'Build-Ubuntu-Clang-arm64-Debug-GN_Android_FrameworkDefs',
      'Build-Ubuntu-Clang-arm64-Release-GN_Android',
      'Build-Ubuntu-Clang-arm64-Release-GN_Android_Vulkan',
      'Build-Ubuntu-Clang-x86_64-Debug-ASAN',
      'Build-Ubuntu-Clang-x86_64-Debug-GN',
      'Build-Ubuntu-Clang-x86_64-Release-Mini',
      'Build-Ubuntu-Clang-x86_64-Release-Vulkan',
      'Build-Ubuntu-GCC-arm-Release-Chromecast',
      'Build-Ubuntu-GCC-x86-Debug',
      'Build-Ubuntu-GCC-x86_64-Debug-GN',
      'Build-Ubuntu-GCC-x86_64-Debug-MSAN',
      'Build-Ubuntu-GCC-x86_64-Debug-NoGPU',
      'Build-Ubuntu-GCC-x86_64-Debug-SK_USE_DISCARDABLE_SCALEDIMAGECACHE',
      'Build-Ubuntu-GCC-x86_64-Release-ANGLE',
      'Build-Ubuntu-GCC-x86_64-Release-Fast',
      'Build-Ubuntu-GCC-x86_64-Release-Flutter_Android',
      'Build-Ubuntu-GCC-x86_64-Release-Mesa',
      'Build-Ubuntu-GCC-x86_64-Release-PDFium',
      'Build-Ubuntu-GCC-x86_64-Release-PDFium_SkiaPaths',
      'Build-Ubuntu-GCC-x86_64-Release-Shared',
      'Build-Ubuntu-GCC-x86_64-Release-Valgrind',
      'Build-Win-Clang-arm64-Release-GN_Android',
      'Build-Win-MSVC-x86-Debug',
      'Build-Win-MSVC-x86-Debug-ANGLE',
      'Build-Win-MSVC-x86-Debug-Exceptions',
      'Build-Win-MSVC-x86-Release-GDI',
      'Build-Win-MSVC-x86-Release-GN',
      'Build-Win-MSVC-x86_64-Release-Vulkan',
    ],
  },
}


def RunSteps(api):
  api.compile.run()


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
                         repository='https://skia.googlesource.com/skia.git',
                         revision='abc123',
                         path_config='kitchen',
                         swarm_out_dir='[SWARM_OUT_DIR]') +
          api.path.exists(
              api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
          )
        )
        if 'Win' in builder:
          test += api.platform('win', 64)
        elif 'Mac' in builder:
          test += api.platform('mac', 64)
        else:
          test += api.platform('linux', 64)
        if 'Trybot' in builder:
          test += api.properties(issue=500,
                                 patchset=1,
                                 rietveld='https://codereview.chromium.org')

        yield test

  mastername = 'client.skia.compile'
  slavename = 'skiabot-win-compile-000'
  buildername = 'Build-Win-MSVC-x86-Debug'
  yield (
      api.test('big_issue_number') +
      api.properties(buildername=buildername,
                     mastername=mastername,
                     slavename=slavename,
                     buildnumber=5,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     rietveld='https://codereview.chromium.org',
                     patchset=1,
                     issue=2147533002L) +
      api.path.exists(
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      ) +
      api.platform('win', 64)
  )

  yield (
      api.test('recipe_with_gerrit_patch') +
      api.properties(
          buildername=buildername + '-Trybot',
          mastername=mastername,
          slavename=slavename,
          buildnumber=5,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          repository='https://skia.googlesource.com/skia.git',
          revision='abc123',
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername + '-Trybot',
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      ) +
      api.platform('win', 64)
  )

  yield (
      api.test('buildbotless_trybot_rietveld') +
      api.properties(
          buildername=buildername,
          mastername=mastername,
          slavename=slavename,
          buildnumber=5,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          repository='https://skia.googlesource.com/skia.git',
          revision='abc123',
          nobuildbot='True',
          issue=500,
          patchset=1,
          patch_storage='rietveld',
          rietveld='https://codereview.chromium.org') +
      api.platform('win', 64)
  )

  yield (
      api.test('buildbotless_trybot_gerrit') +
      api.properties(
          repository='https://skia.googlesource.com/skia.git',
          buildername=buildername,
          mastername=mastername,
          slavename=slavename,
          buildnumber=5,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          revision='abc123',
          nobuildbot='True',
          patch_issue=500,
          patch_set=1,
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      ) +
      api.platform('win', 64)
  )

  buildername = 'Build-Win-MSVC-x86_64-Release-Vulkan'
  yield (
      api.test('alternate_repo') +
      api.properties(buildername=buildername,
                     mastername=mastername,
                     slavename=slavename,
                     buildnumber=5,
                     repository='https://skia.googlesource.com/other_repo.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      )
    )

  buildername = 'Build-Ubuntu-GCC-x86_64-Release-PDFium'
  yield (
      api.test('pdfium_trybot') +
      api.properties(
          repository='https://skia.googlesource.com/skia.git',
          buildername=buildername,
          mastername=mastername,
          slavename=slavename,
          buildnumber=5,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          revision='abc123',
          nobuildbot='True',
          patch_issue=500,
          patch_set=1,
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      ) +
      api.path.exists(
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      )
  )

  buildername = 'Build-Ubuntu-GCC-x86_64-Release-Flutter_Android'
  yield (
      api.test('flutter_trybot') +
      api.properties(
          repository='https://skia.googlesource.com/skia.git',
          buildername=buildername,
          mastername=mastername,
          slavename=slavename,
          buildnumber=5,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          revision='abc123',
          nobuildbot='True',
          patch_issue=500,
          patch_set=1,
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      ) +
      api.path.exists(
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      )
  )
