# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'flavor',
  'recipe_engine/properties',
  'recipe_engine/raw_io',
  'run',
  'vars',
]


def test_exceptions(api):
  try:
    api.flavor.copy_directory_contents_to_device('src', 'dst')
  except ValueError:
    pass
  try:
    api.flavor.copy_directory_contents_to_host('src', 'dst')
  except ValueError:
    pass
  try:
    api.flavor.copy_file_to_device('src', 'dst')
  except ValueError:
    pass


def RunSteps(api):
  api.vars.setup()
  api.flavor.setup()

  if api.properties.get('is_testing_exceptions') == 'True':
    return test_exceptions(api)

  api.flavor.compile('dm')
  api.flavor.copy_extra_build_products(api.vars.swarming_out_dir)
  assert str(api.flavor.out_dir) != ''
  if 'Build' not in api.properties['buildername']:
    try:
      api.flavor.copy_file_to_device('file.txt', 'file.txt')
      api.flavor.create_clean_host_dir('results_dir')
      api.flavor.create_clean_device_dir('device_results_dir')
      api.flavor.install_everything()
      if 'Test' in api.properties['buildername']:
        api.flavor.step('dm', ['dm', '--some-flag'])
        api.flavor.copy_directory_contents_to_host(
            api.flavor.device_dirs.dm_dir, api.vars.dm_dir)
      elif 'Perf' in api.properties['buildername']:
        api.flavor.step('nanobench', ['nanobench', '--some-flag'])
        api.flavor.copy_directory_contents_to_host(
            api.flavor.device_dirs.perf_data_dir, api.vars.perf_data_dir)
    finally:
      api.flavor.cleanup_steps()
  api.run.check_failure()


TEST_BUILDERS = [
  'Build-Debian9-Clang-arm-Release-Android_API26',
  'Build-Debian9-Clang-arm-Release-Chromebook_GLES',
  'Build-Debian9-Clang-arm-Release-Android_ASAN',
  'Build-Debian9-Clang-arm64-Release-Android_ASAN',
  'Build-Debian9-Clang-x86_64-Debug-Chromebook_GLES',
  'Build-Debian9-Clang-x86_64-Debug-SK_USE_DISCARDABLE_SCALEDIMAGECACHE',
  'Build-Debian9-Clang-x86_64-Release-Fast',
  'Build-Debian9-Clang-x86_64-Release-Mini',
  'Build-Debian9-Clang-x86_64-Release-NoDEPS',
  'Build-Debian9-Clang-x86_64-Release-Vulkan',
  'Build-Debian9-EMCC-wasm-Release',
  'Build-Debian9-GCC-x86_64-Debug-EmbededResouces',
  'Build-Debian9-GCC-x86_64-Release-ANGLE',
  'Build-Debian9-GCC-x86_64-Release-Flutter_Android',
  'Build-Debian9-GCC-x86_64-Release-NoGPU',
  'Build-Debian9-GCC-x86_64-Release-PDFium',
  'Build-Debian9-GCC-x86_64-Release-PDFium_SkiaPaths',
  'Build-Debian9-GCC-x86_64-Release-Shared',
  'Build-Mac-Clang-arm64-Debug-Android_Vulkan',
  'Build-Mac-Clang-arm64-Debug-iOS',
  'Build-Mac-Clang-x86_64-Debug-CommandBuffer',
  'Build-Mac-Clang-x86_64-Debug-Metal',
  'Build-Win-Clang-arm64-Release-Android',
  'Build-Win-Clang-x86_64-Debug-GDI',
  'Build-Win-Clang-x86_64-Debug-GomaNoFallback',
  'Build-Win-Clang-x86_64-Release',
  'Build-Win-Clang-x86_64-Release-ANGLE_Goma',
  'Build-Win-Clang-x86_64-Release-Goma',
  'Build-Win-Clang-x86_64-Release-Vulkan',
  'Build-Win-MSVC-x86-Debug-Exceptions',
  'Housekeeper-PerCommit-CheckGeneratedFiles',
  'Perf-Android-Clang-GalaxyS7_G930FD-GPU-MaliT880-arm64-Debug-All-Android',
  'Perf-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Debug-All-Android',
  'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-All-Android',
  'Perf-Android-Clang-Pixel-GPU-Adreno530-arm64-Debug-All-Android',
  'Perf-ChromeOS-Clang-SamsungChromebookPlus-GPU-MaliT860-arm-Release-All',
  'Perf-Chromecast-GCC-Chorizo-CPU-Cortex_A7-arm-Release-All',
  'Perf-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-MSAN',
  'Perf-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All-ASAN',
  'Perf-Ubuntu14-GCC-GCE-CPU-AVX2-x86_64-Release-All-CT_BENCH_1k_SKPs',
  'Test-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-Android',
  'Test-Android-Clang-GalaxyS7_G930FD-GPU-MaliT880-arm64-Debug-All-Android',
  'Test-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Debug-All-Android',
  'Test-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Release-All-Android_ASAN',
  'Test-Android-Clang-Nexus7-CPU-Tegra3-arm-Release-All-Android',
  'Test-Android-Clang-Pixel-GPU-Adreno530-arm64-Debug-All-Android',
  'Test-ChromeOS-Clang-SamsungChromebookPlus-GPU-MaliT860-arm-Release-All',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-Coverage',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All-TSAN',
  'Test-Debian9-GCC-GCE-CPU-AVX2-x86_64-Release-All',
  'Test-Ubuntu16-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All-Vulkan',
  ('Test-Ubuntu17-GCC-Golo-GPU-QuadroP400-x86_64-Release-All'
   '-Valgrind_AbandonGpuContext_SK_CPU_LIMIT_SSE41'),
  'Test-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Debug-All',
  'Test-iOS-Clang-iPadPro-GPU-GT7800-arm64-Debug-All',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-SafeStack',
]

# Default properties used for TEST_BUILDERS.
defaultProps = lambda buildername: dict(
  buildername=buildername,
  repository='https://skia.googlesource.com/skia.git',
  revision='abc123',
  path_config='kitchen',
  patch_set=2,
  swarm_out_dir='[SWARM_OUT_DIR]'
)

def GenTests(api):
  for buildername in TEST_BUILDERS:
    test = (
      api.test(buildername) +
      api.properties(**defaultProps(buildername))
    )
    if 'Chromebook' in buildername and not 'Build' in buildername:
      test += api.step_data(
          'read chromeos ip',
          stdout=api.raw_io.output('{"user_ip":"foo@127.0.0.1"}'))
    if 'Chromecast' in buildername:
      test += api.step_data(
          'read chromecast ip',
          stdout=api.raw_io.output('192.168.1.2:5555'))
    yield test

  builder = 'Test-Debian9-GCC-GCE-CPU-AVX2-x86_64-Release-All'
  yield (
      api.test('exceptions') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     is_testing_exceptions='True')
  )

  builder = 'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-All-Android'
  yield (
      api.test('failed_infra_step') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('get swarming bot id',
                    stdout=api.raw_io.output('build123-m2--device5')) +
      api.step_data('dump log', retcode=1)
  )

  builder = 'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-All-Android'
  yield (
      api.test('failed_read_version') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('read /sdcard/revenge_of_the_skiabot/SK_IMAGE_VERSION',
                    retcode=1)
  )

  builder = 'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-All-Android'
  yield (
      api.test('retry_adb_command') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('mkdir /sdcard/revenge_of_the_skiabot/resources',
                    retcode=1)
  )

  builder = 'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-All-Android'
  fail_step_name = 'mkdir /sdcard/revenge_of_the_skiabot/resources'
  yield (
      api.test('retry_adb_command_retries_exhausted') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('get swarming bot id',
                    stdout=api.raw_io.output('build123-m2--device5')) +
      api.step_data(fail_step_name, retcode=1) +
      api.step_data(fail_step_name + ' (attempt 2)', retcode=1) +
      api.step_data(fail_step_name + ' (attempt 3)', retcode=1)
  )

  yield (
      api.test('cpu_scale_failed') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('Scale CPU 0 to 0.600000', retcode=1)
  )

  builder = 'Test-iOS-Clang-iPhone7-GPU-GT7600-arm64-Release-All'
  fail_step_name = 'install_dm'
  yield (
      api.test('retry_ios_install') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data(fail_step_name, retcode=1)
  )

  yield (
      api.test('retry_ios_install_retries_exhausted') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data(fail_step_name, retcode=1) +
      api.step_data(fail_step_name + ' (attempt 2)', retcode=1)
  )
