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
      api.flavor.step('dm', ['dm', '--some-flag'])
      api.flavor.copy_directory_contents_to_host(
          api.flavor.device_dirs.dm_dir, api.vars.dm_dir)
      api.flavor.copy_directory_contents_to_host(
          api.flavor.device_dirs.perf_data_dir, api.vars.perf_data_dir)
    finally:
      api.flavor.cleanup_steps()
  api.run.check_failure()


TEST_BUILDERS = [
  'Build-Mac-Clang-arm64-Debug-Android_Vulkan',
  'Build-Mac-Clang-x86_64-Debug-CommandBuffer',
  'Build-Ubuntu-Clang-x86_64-Release-Mini',
  'Build-Ubuntu-Clang-x86_64-Release-Shared',
  'Build-Ubuntu-Clang-x86_64-Release-Vulkan',
  'Build-Ubuntu-GCC-x86_64-Debug-SK_USE_DISCARDABLE_SCALEDIMAGECACHE',
  'Build-Ubuntu-GCC-x86_64-Release-ANGLE',
  'Build-Ubuntu-GCC-x86_64-Release-Fast',
  'Build-Ubuntu-GCC-x86_64-Release-Flutter_Android',
  'Build-Ubuntu-GCC-x86_64-Release-Mesa',
  'Build-Ubuntu-GCC-x86_64-Release-PDFium',
  'Build-Ubuntu-GCC-x86_64-Release-PDFium_SkiaPaths',
  'Build-Win-Clang-arm64-Release-Android',
  'Build-Win-MSVC-x86_64-Debug-GDI',
  'Build-Win-MSVC-x86_64-Debug-NoGPU',
  'Build-Win-MSVC-x86_64-Release-Exceptions',
  'Build-Win-MSVC-x86_64-Release-Vulkan',
  'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-Android',
  'Perf-ChromeOS-Clang-Chromebook_513C24_K01-GPU-MaliT860-arm-Release',
  'Perf-Chromecast-GCC-Chorizo-CPU-Cortex_A7-arm-Release',
  'Perf-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Release-ASAN',
  'Perf-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Release-MSAN',
  'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release',
  ('Test-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-'
   'Valgrind_AbandonGpuContext'),
  'Test-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Debug',
  'Test-iOS-Clang-iPadMini4-GPU-GX6450-arm64-Debug',
]


def GenTests(api):
  for buildername in TEST_BUILDERS:
    test = (
      api.test(buildername) +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
    )
    if 'Chromebook' in buildername:
      test += api.step_data(
          'read chromeos ip',
          stdout=api.raw_io.output('{"user_ip":"foo@127.0.0.1"}'))
    if 'Chromecast' in buildername:
      test += api.step_data(
          'read chromecast ip',
          stdout=api.raw_io.output('192.168.1.2:5555'))
    yield test

  builder = 'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release'
  yield (
      api.test('exceptions') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     is_testing_exceptions='True')
  )

  builder = 'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-Android'
  yield (
      api.test('failed_infra_step') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('dump log', retcode=1)
  )

  builder = 'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-Android'
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
