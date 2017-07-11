# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming compile.


DEPS = [
  'core',
  'recipe_engine/context',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
  'flavor',
  'run',
  'vars',
]


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


def RunSteps(api):
  api.core.setup()

  env = get_extra_env_vars(api.vars.builder_cfg)
  build_targets = build_targets_from_builder_dict(api.vars.builder_cfg)

  try:
    for target in build_targets:
      with api.context(env=env):
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


TEST_BUILDERS = [
  'Build-Mac-Clang-arm64-Debug-Android',
  'Build-Mac-Clang-arm64-Debug-iOS',
  'Build-Mac-Clang-x64-Release-iOS',
  'Build-Mac-Clang-x86_64-Debug-CommandBuffer',
  'Build-Mac-Clang-x86_64-Release',
  'Build-Ubuntu-Clang-arm-Release-Chromebook_C100p',
  'Build-Ubuntu-Clang-arm64-Release-Android',
  'Build-Ubuntu-Clang-arm64-Release-Android_Vulkan',
  'Build-Ubuntu-Clang-mipsel-Debug-Android',
  'Build-Ubuntu-Clang-x86_64-Debug',
  'Build-Ubuntu-Clang-x86_64-Debug-ASAN',
  'Build-Ubuntu-Clang-x86_64-Debug-MSAN',
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
  'Build-Win-Clang-arm64-Release-Android',
  'Build-Win-MSVC-x86-Debug',
  'Build-Win-MSVC-x86-Debug-ANGLE',
  'Build-Win-MSVC-x86-Debug-Exceptions',
  'Build-Win-MSVC-x86-Release-GDI',
  'Build-Win-MSVC-x86-Release-GN',
  'Build-Win-MSVC-x86_64-Release-Vulkan',
]


def GenTests(api):
  for builder in TEST_BUILDERS:
    test = (
      api.test(builder) +
      api.properties(buildername=builder,
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

    yield test


  buildername = 'Build-Win-MSVC-x86_64-Release-Vulkan'
  yield (
      api.test("trybot") +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      ) +
      api.properties(patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=buildername,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )
    )

  buildername = 'Build-Win-MSVC-x86_64-Release-Vulkan'
  yield (
      api.test('alternate_repo') +
      api.properties(buildername=buildername,
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
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          revision='abc123',
          patch_issue=500,
          patch_repo='https://skia.googlesource.com/skia.git',
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
          path_config='kitchen',
          swarm_out_dir='[SWARM_OUT_DIR]',
          revision='abc123',
          patch_issue=500,
          patch_repo='https://skia.googlesource.com/skia.git',
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
