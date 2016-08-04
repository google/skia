# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming compile.


DEPS = [
  'core',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/python',
  'flavor',
  'run',
  'vars',
]


TEST_BUILDERS = {
  'client.skia.compile': {
    'skiabot-linux-swarm-000': [
      'Build-Mac-Clang-Arm7-Debug-Android',
      'Build-Mac-Clang-Arm7-Release-iOS',
      'Build-Mac-Clang-x86_64-Debug-CommandBuffer',
      'Build-Mac-Clang-x86_64-Release-CMake',
      'Build-Ubuntu-Clang-x86_64-Debug-GN',
      'Build-Ubuntu-GCC-Arm7-Debug-Android-Trybot',
      'Build-Ubuntu-GCC-Arm7-Release-Android',
      'Build-Ubuntu-GCC-Arm7-Release-Android_Vulkan',
      'Build-Ubuntu-GCC-x86-Debug',
      'Build-Ubuntu-GCC-x86_64-Debug-MSAN',
      'Build-Ubuntu-GCC-x86_64-Debug-GN',
      'Build-Ubuntu-GCC-x86_64-Release-CMake',
      'Build-Ubuntu-GCC-x86_64-Release-PDFium',
      'Build-Ubuntu-GCC-x86_64-Release-Shared',
      'Build-Ubuntu-GCC-x86_64-Release-Valgrind',
      'Build-Win-MSVC-x86-Debug',
      'Build-Win-MSVC-x86-Release-GN',
      'Build-Win-MSVC-x86_64-Release-Vulkan',
    ],
  },
}


def RunSteps(api):
  api.core.setup()

  try:
    for target in api.vars.build_targets:
      api.flavor.compile(target)
    api.run.copy_build_products(
        api.flavor.out_dir,
        api.vars.swarming_out_dir.join(
            'out', api.vars.configuration))
    api.flavor.copy_extra_build_products(api.vars.swarming_out_dir)
  finally:
    if 'Win' in api.vars.builder_cfg.get('os', ''):
      api.python.inline(
          name='cleanup',
          program='''import psutil
for p in psutil.process_iter():
  try:
    if p.name in ('mspdbsrv.exe', 'vctip.exe', 'cl.exe', 'link.exe'):
      p.kill()
  except psutil._error.AccessDenied:
    pass
''',
          infra_step=True)

  api.flavor.cleanup_steps()
  api.run.check_failure()


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
              api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
          )
        )
        if 'Win' in builder:
          test += api.platform('win', 64)
        elif 'Mac' in builder:
          test += api.platform('mac', 64)
        else:
          test += api.platform('linux', 64)
        if 'Android' in builder or ('GN' in builder and 'Win' not in builder):
          ccache = '/usr/bin/ccache'
          test += api.step_data('has ccache?',
                                stdout=api.json.output({'ccache':ccache}))
        if 'Android' in builder:
          test += api.step_data(
            'which adb',
            retcode=1)
        if 'Trybot' in builder:
          test += api.properties(issue=500,
                                 patchset=1,
                                 rietveld='https://codereview.chromium.org')

        yield test

  mastername = 'client.skia.compile'
  slavename = 'skiabot-win-compile-000'
  buildername = 'Build-Ubuntu-GCC-x86-Debug'
  yield (
      api.test('failed_compile') +
      api.properties(buildername=buildername,
                     mastername=mastername,
                     slavename=slavename,
                     buildnumber=5,
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(
          api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
      ) +
      api.step_data('build most', retcode=1)
  )

  buildername = 'Build-Win-MSVC-x86-Debug'
  yield (
      api.test('win_retry_failed_compile') +
      api.properties(buildername=buildername,
                     mastername=mastername,
                     slavename=slavename,
                     buildnumber=5,
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(
          api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
      ) +
      api.platform('win', 64) +
      api.step_data('build most', retcode=1)
  )

  yield (
      api.test('big_issue_number') +
      api.properties(buildername=buildername,
                     mastername=mastername,
                     slavename=slavename,
                     buildnumber=5,
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     rietveld='https://codereview.chromium.org',
                     patchset=1,
                     issue=2147533002L) +
      api.path.exists(
          api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
      ) +
      api.platform('win', 64)
  )
