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
      'Build-Mac-Clang-mipsel-Debug-GN_Android',
      'Build-Mac-Clang-x86_64-Debug-CommandBuffer',
      'Build-Mac-Clang-x86_64-Release-CMake',
      'Build-Mac-Clang-x86_64-Release-GN',
      'Build-Ubuntu-Clang-arm64-Release-GN_Android',
      'Build-Ubuntu-Clang-x86_64-Debug-GN',
      'Build-Ubuntu-GCC-Arm7-Debug-Android-Trybot',
      'Build-Ubuntu-GCC-Arm7-Debug-Android_FrameworkDefs',
      'Build-Ubuntu-GCC-Arm7-Debug-Android_NoNeon',
      'Build-Ubuntu-GCC-Arm7-Release-Android',
      'Build-Ubuntu-GCC-Arm7-Release-Android_Vulkan',
      'Build-Ubuntu-GCC-x86-Debug',
      'Build-Ubuntu-GCC-x86_64-Debug-GN',
      'Build-Ubuntu-GCC-x86_64-Debug-MSAN',
      'Build-Ubuntu-GCC-x86_64-Debug-SK_USE_DISCARDABLE_SCALEDIMAGECACHE',
      'Build-Ubuntu-GCC-x86_64-Release-ANGLE',
      'Build-Ubuntu-GCC-x86_64-Release-CMake',
      'Build-Ubuntu-GCC-x86_64-Release-Fast',
      'Build-Ubuntu-GCC-x86_64-Release-Mesa',
      'Build-Ubuntu-GCC-x86_64-Release-PDFium',
      'Build-Ubuntu-GCC-x86_64-Release-Shared',
      'Build-Ubuntu-GCC-x86_64-Release-Valgrind',
      'Build-Win-MSVC-x86-Debug',
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
  if 'SAN' in builder_dict.get('extra_config', ''):
    # 'most' does not compile under MSAN.
    return ['dm', 'nanobench']
  else:
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

  # skia_arch_type.
  arch = builder_dict['target_arch']

  arch_types = {
    'x86':      'x86',
    'x86_64':   'x86_64',
    'Arm7':     'arm',
    'Arm64':    'arm64',
    'Mips':     'mips32',
    'Mips64':   'mips64',
    'MipsDSP2': 'mips32',
  }
  if arch in arch_types:
    gyp_defs['skia_arch_type']  = arch_types[arch]

  # skia_warnings_as_errors.
  werr = False
  if 'Win' in builder_dict.get('os', ''):
    if not ('GDI' in builder_dict.get('extra_config', '') or
            'Exceptions' in builder_dict.get('extra_config', '')):
      werr = True
  elif ('Mac' in builder_dict.get('os', '') and
        'Android' in builder_dict.get('extra_config', '')):
    werr = False
  elif 'Fast' in builder_dict.get('extra_config', ''):
    # See https://bugs.chromium.org/p/skia/issues/detail?id=5257
    werr = False
  else:
    werr = True
  gyp_defs['skia_warnings_as_errors'] = str(int(werr))  # True/False -> '1'/'0'

  # Win debugger.
  if 'Win' in builder_dict.get('os', ''):
    gyp_defs['skia_win_debuggers_path'] = 'c:/DbgHelp'

  # Qt SDK (Win).
  if 'Win' in builder_dict.get('os', ''):
    gyp_defs['qt_sdk'] = 'C:/Qt/4.8.5/'

  # ANGLE.
  if builder_dict.get('extra_config') == 'ANGLE':
    gyp_defs['skia_angle'] = '1'
    if builder_dict.get('os', '') in ('Ubuntu', 'Linux'):
      gyp_defs['use_x11'] = '1'
      gyp_defs['chromeos'] = '0'

  # GDI.
  if builder_dict.get('extra_config') == 'GDI':
    gyp_defs['skia_gdi'] = '1'

  # Build with Exceptions on Windows.
  if ('Win' in builder_dict.get('os', '') and
      builder_dict.get('extra_config') == 'Exceptions'):
    gyp_defs['skia_win_exceptions'] = '1'

  # iOS.
  if (builder_dict.get('os') == 'iOS' or
      builder_dict.get('extra_config') == 'iOS'):
    gyp_defs['skia_os'] = 'ios'

  # Shared library build.
  if builder_dict.get('extra_config') == 'Shared':
    gyp_defs['skia_shared_lib'] = '1'

  # Build fastest Skia possible.
  if builder_dict.get('extra_config') == 'Fast':
    gyp_defs['skia_fast'] = '1'

  # Clang.
  if builder_dict.get('compiler') == 'Clang':
    gyp_defs['skia_clang_build'] = '1'

  # Valgrind.
  if 'Valgrind' in builder_dict.get('extra_config', ''):
    gyp_defs['skia_release_optimization_level'] = '1'

  # Link-time code generation just wastes time on compile-only bots.
  if builder_dict.get('compiler') == 'MSVC':
    gyp_defs['skia_win_ltcg'] = '0'

  # Mesa.
  if (builder_dict.get('extra_config') == 'Mesa' or
      builder_dict.get('cpu_or_gpu_value') == 'Mesa'):
    gyp_defs['skia_mesa'] = '1'

  # skia_use_android_framework_defines.
  if builder_dict.get('extra_config') == 'Android_FrameworkDefs':
    gyp_defs['skia_use_android_framework_defines'] = '1'

  # CommandBuffer.
  if builder_dict.get('extra_config') == 'CommandBuffer':
    gyp_defs['skia_command_buffer'] = '1'

  # Vulkan.
  if builder_dict.get('extra_config') == 'Vulkan':
    gyp_defs['skia_vulkan'] = '1'
    gyp_defs['skia_vulkan_debug_layers'] = '0'

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
        if 'Android' in builder or ('GN' in builder and 'Win' not in builder):
          ccache = '/usr/bin/ccache'
          test += api.step_data('has ccache?',
                                stdout=api.json.output({'ccache':ccache}))
        if 'Android' in builder and 'GN_Android' not in builder:
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

  gerrit_kwargs = {
    'patch_storage': 'gerrit',
    'repository': 'skia',
    'event.patchSet.ref': 'refs/changes/00/2100/2',
    'event.change.number': '2100',
  }
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
          **gerrit_kwargs) +
      api.platform('win', 64)
  )
