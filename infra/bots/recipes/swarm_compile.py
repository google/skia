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
      'Build-Mac-Clang-Arm7-Release-iOS',
      'Build-Mac-Clang-arm64-Debug-GN_iOS',
      'Build-Mac-Clang-mipsel-Debug-GN_Android',
      'Build-Mac-Clang-x86_64-Debug-CommandBuffer',
      'Build-Mac-Clang-x86_64-Release-GN',
      'Build-Ubuntu-Clang-arm64-Release-GN_Android',
      'Build-Ubuntu-Clang-arm64-Release-GN_Android_Vulkan',
      'Build-Ubuntu-Clang-x86_64-Debug-ASAN',
      'Build-Ubuntu-Clang-x86_64-Debug-GN',
      'Build-Ubuntu-Clang-arm64-Debug-GN_Android-Trybot',
      'Build-Ubuntu-Clang-arm64-Debug-GN_Android_FrameworkDefs',
      'Build-Ubuntu-GCC-x86-Debug',
      'Build-Ubuntu-GCC-x86_64-Debug-GN',
      'Build-Ubuntu-GCC-x86_64-Debug-MSAN',
      'Build-Ubuntu-GCC-x86_64-Debug-NoGPU',
      'Build-Ubuntu-GCC-x86_64-Debug-SK_USE_DISCARDABLE_SCALEDIMAGECACHE',
      'Build-Ubuntu-GCC-x86_64-Release-ANGLE',
      'Build-Ubuntu-GCC-x86_64-Release-Fast',
      'Build-Ubuntu-GCC-x86_64-Release-Mesa',
      'Build-Ubuntu-GCC-x86_64-Release-PDFium',
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


def build_targets_from_builder_dict(builder_dict):
  """Return a list of targets to build, depending on the builder type."""
  if builder_dict.get('extra_config') == 'iOS':
    return ['iOSShell']
  return ['most']


def get_extra_env_vars(builder_dict):
  env = {}
  if builder_dict.get('compiler') == 'Clang':
    env['CC'] = '/usr/bin/clang'
    env['CXX'] = '/usr/bin/clang++'

  # SKNX_NO_SIMD, SK_USE_DISCARDABLE_SCALEDIMAGECACHE, etc.
  extra_config = builder_dict.get('extra_config', '')
  if extra_config.startswith('SK') and extra_config.isupper():
    env['CPPFLAGS'] = '-D' + extra_config

  return env


def get_gyp_defines(builder_dict):
  gyp_defs = {}

  if (builder_dict.get('os') == 'iOS' or
      builder_dict.get('extra_config') == 'iOS'):
    gyp_defs['skia_arch_type']  = 'arm'
    gyp_defs['skia_clang_build'] = '1'
    gyp_defs['skia_os'] = 'ios'
    gyp_defs['skia_warnings_as_errors'] = 1

  return gyp_defs


def RunSteps(api):
  api.core.setup()

  env = get_extra_env_vars(api.vars.builder_cfg)
  gyp_defs = get_gyp_defines(api.vars.builder_cfg)
  gyp_defs_list = ['%s=%s' % (k, v) for k, v in gyp_defs.iteritems()]
  gyp_defs_list.sort()
  env['GYP_DEFINES'] = ' '.join(gyp_defs_list)

  build_targets = build_targets_from_builder_dict(api.vars.builder_cfg)

  try:
    for target in build_targets:
      api.flavor.compile(target, env=env)
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

  yield (
      api.test('recipe_with_gerrit_patch') +
      api.properties(
          buildername=buildername + '-Trybot',
          mastername=mastername,
          slavename=slavename,
          buildnumber=5,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
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
          repository='skia',
          buildername=buildername,
          mastername=mastername,
          slavename=slavename,
          buildnumber=5,
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
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
          repository='skia',
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
