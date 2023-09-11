# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = "PY3"

DEPS = [
  'flavor',
  'recipe_engine/platform',
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

  builder = api.properties['buildername']
  app = None
  if 'SkottieTracing' in builder:
    app = None
  elif 'Test' in builder:
    app = 'dm'
  elif 'Perf' in builder:
    app = 'nanobench'
  api.flavor.setup(app)

  if api.properties.get('is_testing_exceptions') == 'True':
    return test_exceptions(api)

  try:
    api.flavor.copy_file_to_device('file.txt', 'file.txt')
    api.flavor.read_file_on_device('file.txt')
    api.flavor.remove_file_on_device('file.txt')
    api.flavor.create_clean_host_dir('results_dir')
    api.flavor.create_clean_device_dir('device_results_dir')

    if 'Lottie' in builder:
      api.flavor.install(lotties=True)
    elif 'Mskp' in builder:
      api.flavor.install(mskps=True)
    elif all(v in builder for v in ['Perf', 'Android', 'CPU']):
      api.flavor.install(skps=True, images=True, svgs=True,
                         resources=True, texttraces=True)
    else:
      api.flavor.install(skps=True, images=True, lotties=False,
                         svgs=True, resources=True)
    if 'Test' in builder:
      api.flavor.step('dm', ['dm', '--some-flag'])
      api.flavor.copy_directory_contents_to_host(
          api.flavor.device_dirs.dm_dir, api.flavor.host_dirs.dm_dir)
    elif 'Perf' in builder:
      if 'SkottieTracing' in builder:
        api.flavor.step('dm', ['dm', '--some-flag'])
      else:
        api.flavor.step('nanobench', ['nanobench', '--some-flag'])
      api.flavor.copy_directory_contents_to_host(
          api.flavor.device_dirs.perf_data_dir,
          api.flavor.host_dirs.perf_data_dir)
  finally:
    api.flavor.cleanup_steps()
  api.run.check_failure()


TEST_BUILDERS = [
  'Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-Android_SkottieTracing',
  'Perf-Android-Clang-GalaxyS7_G930FD-GPU-MaliT880-arm64-Debug-All-Android',
  'Perf-Android-Clang-NVIDIA_Shield-CPU-TegraX1-arm64-Release-All-Android',
  'Perf-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Debug-All-Android',
  'Perf-Android-Clang-Pixel2XL-GPU-Adreno540-arm64-Release-All-Android_Skpbench_Mskp',
  'Perf-Android-Clang-GalaxyS20-GPU-MaliG77-arm64-Release-All-Android_Vulkan',
  'Perf-Android-Clang-Pixel6-GPU-Adreno620-arm64-Release-All-Android',
  'Perf-ChromeOS-Clang-SamsungChromebookPlus-GPU-MaliT860-arm-Release-All',
  'Perf-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All-MSAN',
  'Perf-Debian10-Clang-GCE-CPU-AVX2-x86_64-Release-All-ASAN',
  'Perf-Win2019-Clang-GCE-CPU-AVX2-x86_64-Debug-All-ASAN',
  'Test-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-Android',
  'Test-Android-Clang-GalaxyS7_G930FD-GPU-MaliT880-arm64-Debug-All-Android',
  'Test-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Debug-All-Android',
  'Test-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Release-All-Android_ASAN',
  'Test-Android-Clang-Pixel3a-GPU-Adreno615-arm64-Debug-All-Android_Vulkan',
  'Test-ChromeOS-Clang-SamsungChromebookPlus-GPU-MaliT860-arm-Release-All',
  'Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Debug-All-Coverage',
  'Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Release-All-Lottie',
  'Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Release-All-TSAN',
  'Test-Debian10-Clang-GCE-GPU-SwiftShader-x86_64-Debug-All-SwiftShader',
  'Test-Debian10-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All-Vulkan',
  'Test-Mac10.13-Clang-MacBookPro11.5-CPU-AVX2-x86_64-Debug-All-ASAN',
  ('Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All'
   '-Valgrind_AbandonGpuContext_SK_CPU_LIMIT_SSE41'),
  'Test-Debian10-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All-ASAN_Vulkan',
  'Test-Debian11-Clang-NUC11TZi5-GPU-IntelIrisXe-x86_64-Debug-All',
  'Test-Win10-Clang-NUC5i7RYH-CPU-AVX2-x86_64-Debug-All-NativeFonts_DWriteCore',
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
    if 'Win' in buildername:
      test += api.platform('win', 64)
    yield test

  builder = 'Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Release-All'
  yield (
      api.test('exceptions') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     is_testing_exceptions='True')
  )

  builder = ('Perf-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Debug-All'
             '-Android')
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

  builder = 'Test-iOS-Clang-iPhone7-GPU-PowerVRGT7600-arm64-Release-All'
  fail_step_name = 'install dm'
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
  fail_step_name = 'dm'
  yield (
      api.test('ios_rerun_with_debug') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data(fail_step_name, retcode=1)
  )

  builder = ('Perf-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Debug-All'
             '-Android')
  yield (
    api.test('cpu_scale_failed_once') +
    api.properties(buildername=builder,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
    api.step_data('Scale CPU 4 to 0.600000', retcode=1)
  )

  yield (
    api.test('cpu_scale_failed') +
    api.properties(buildername=builder,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
    api.step_data('get swarming bot id',
                  stdout=api.raw_io.output('skia-rpi-022')) +
    api.step_data('Scale CPU 4 to 0.600000', retcode=1)+
    api.step_data('Scale CPU 4 to 0.600000 (attempt 2)', retcode=1)+
    api.step_data('Scale CPU 4 to 0.600000 (attempt 3)', retcode=1)
  )

  builder = ('Perf-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Release'
             '-All-Android')
  yield (
    api.test('cpu_scale_failed_golo') +
    api.properties(buildername=builder,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
    api.step_data('get swarming bot id',
                  stdout=api.raw_io.output('build123-m2--device5')) +
    api.step_data('Scale CPU 4 to 0.600000', retcode=1)+
    api.step_data('Scale CPU 4 to 0.600000 (attempt 2)', retcode=1)+
    api.step_data('Scale CPU 4 to 0.600000 (attempt 3)', retcode=1)
  )
