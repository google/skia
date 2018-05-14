# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming compile.


DEPS = [
  'build',
  'core',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
  'run',
  'vars',
]


def RunSteps(api):
  api.vars.setup()

  # Check out code.
  if 'NoDEPS' in api.properties['buildername']:
    api.core.checkout_git()
  else:
    api.core.checkout_bot_update()
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)

  try:
    api.build()
    api.build.copy_build_products(
        api.vars.swarming_out_dir.join(
            'out', api.vars.configuration))
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

  api.run.check_failure()


TEST_BUILDERS = [
  'Build-Debian9-Clang-arm-Release-Chromebook_GLES',
  'Build-Debian9-Clang-arm64-Release-Android',
  'Build-Debian9-Clang-arm64-Release-Android_Vulkan',
  'Build-Debian9-Clang-arm64-Release-Android_ASAN',
  'Build-Debian9-Clang-x86_64-Debug',
  'Build-Debian9-Clang-x86_64-Debug-ASAN',
  'Build-Debian9-Clang-x86_64-Debug-Coverage',
  'Build-Debian9-Clang-x86_64-Debug-MSAN',
  'Build-Debian9-Clang-x86_64-Debug-SK_USE_DISCARDABLE_SCALEDIMAGECACHE',
  'Build-Debian9-Clang-x86_64-Release-Chromebook_GLES',
  'Build-Debian9-Clang-x86_64-Release-Fast',
  'Build-Debian9-Clang-x86_64-Release-Mini',
  'Build-Debian9-Clang-x86_64-Release-NoDEPS',
  'Build-Debian9-Clang-x86_64-Release-Vulkan',
  'Build-Debian9-Clang-x86_64-Release-Vulkan_Coverage',
  'Build-Debian9-EMCC-wasm-Release',
  'Build-Debian9-GCC-arm-Release-Chromecast',
  'Build-Debian9-GCC-x86-Debug',
  'Build-Debian9-GCC-x86_64-Debug-NoGPU',
  'Build-Debian9-GCC-x86_64-Release-ANGLE',
  'Build-Debian9-GCC-x86_64-Release-Flutter_Android',
  'Build-Debian9-GCC-x86_64-Release-Shared',
  'Build-Mac-Clang-arm64-Debug-Android',
  'Build-Mac-Clang-arm64-Debug-iOS',
  'Build-Mac-Clang-x64-Release-iOS',
  'Build-Mac-Clang-x86_64-Debug-CommandBuffer',
  'Build-Mac-Clang-x86_64-Release',
  'Build-Win-Clang-arm64-Release-Android',
  'Build-Win-Clang-x86-Debug',
  'Build-Win-Clang-x86-Debug-Exceptions',
  'Build-Win-Clang-x86_64-Debug-ANGLE',
  'Build-Win-Clang-x86_64-Release-Vulkan',
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


  buildername = 'Build-Win-Clang-x86_64-Release-Vulkan'
  yield (
      api.test('trybot') +
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

  buildername = 'Build-Debian9-GCC-x86_64-Release-Flutter_Android'
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
