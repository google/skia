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
  os = api.properties['os']
  compiler = api.properties['compiler']
  model = api.properties['model']
  cpu_or_gpu = api.properties['cpu_or_gpu']
  cpu_or_gpu_value = api.properties['cpu_or_gpu_value']
  arch = api.properties['arch']
  configuration = api.properties['configuration']
  test_filter = api.properties['test_filter']
  extra_tokens = api.properties.get('extra_tokens', '').split(',')
  api.flavor.setup(os, compiler, model, cpu_or_gpu, cpu_or_gpu_value, arch,
                   configuration, test_filter, extra_tokens)

  if api.properties.get('is_testing_exceptions') == 'True':
    return test_exceptions(api)

  if 'Build' not in api.properties['buildername']:
    try:
      api.flavor.copy_file_to_device('file.txt', 'file.txt')
      api.flavor.create_clean_host_dir('results_dir')
      api.flavor.create_clean_device_dir('device_results_dir')
      api.flavor.install_everything()
      if 'Test' in api.properties['buildername']:
        api.flavor.step('dm', ['dm', '--some-flag'])
        api.flavor.copy_directory_contents_to_host(
            api.flavor.device_dirs.dm_dir, api.flavor.host_dirs.dm_dir)
      elif 'Perf' in api.properties['buildername']:
        api.flavor.step('nanobench', ['nanobench', '--some-flag'])
        api.flavor.copy_directory_contents_to_host(
            api.flavor.device_dirs.perf_data_dir,
            api.flavor.host_dirs.perf_data_dir)
    finally:
      api.flavor.cleanup_steps()
  api.run.check_failure()


TEST_BUILDERS = [
  'Perf-Android-Clang-GalaxyS7_G930FD-GPU-MaliT880-arm64-Debug-All-Android',
  'Perf-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Debug-All-Android',
  'Perf-ChromeOS-Clang-SamsungChromebookPlus-GPU-MaliT860-arm-Release-All',
  'Perf-Chromecast-GCC-Chorizo-CPU-Cortex_A7-arm-Release-All',
  'Perf-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-MSAN',
  'Perf-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All-ASAN',
  'Test-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-Android',
  'Test-Android-Clang-GalaxyS7_G930FD-GPU-MaliT880-arm64-Debug-All-Android',
  'Test-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Debug-All-Android',
  'Test-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Release-All-Android_ASAN',
  'Test-ChromeOS-Clang-SamsungChromebookPlus-GPU-MaliT860-arm-Release-All',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-Coverage',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All-TSAN',
  'Test-Debian9-Clang-GCE-GPU-SwiftShader-x86_64-Debug-All-SwiftShader',
  'Test-Debian9-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All-Vulkan',
  'Test-Mac-Clang-MacMini7.1-CPU-AVX-x86_64-Debug-All-ASAN',
  ('Test-Ubuntu17-GCC-Golo-GPU-QuadroP400-x86_64-Release-All'
   '-Valgrind_AbandonGpuContext_SK_CPU_LIMIT_SSE41'),
  'Test-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-Vulkan_ProcDump',
]

# Default properties used for TEST_BUILDERS.
def defaultProps(buildername):
  split = buildername.split('-')
  os = split[1]
  compiler = split[2]
  model = split[3]
  cpu_or_gpu = split[4]
  cpu_or_gpu_value = split[5]
  arch = split[6]
  configuration = split[7]
  test_filter = split[8]

  extra_tokens_list = []
  if len(split) > 9:
    extra_split = split[9].split('_')
    for idx, tok in enumerate(extra_split):
      if tok == 'SK':
        extra_tokens_list.append('_'.join(extra_split[idx:]))
        break
      else:
        extra_tokens_list.append(tok)
  extra_tokens = ','.join(extra_tokens_list)

  return dict(
    arch=arch,
    buildername=buildername,
    compiler=compiler,
    configuration=configuration,
    cpu_or_gpu=cpu_or_gpu,
    cpu_or_gpu_value=cpu_or_gpu_value,
    extra_tokens=extra_tokens,
    model=model,
    os=os,
    patch_set=2,
    path_config='kitchen',
    repository='https://skia.googlesource.com/skia.git',
    revision='abc123',
    swarm_out_dir='[SWARM_OUT_DIR]',
    test_filter=test_filter
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
      api.properties(arch='x86_64',
                     buildername=builder,
                     compiler='GCC',
                     configuration='Release',
                     cpu_or_gpu='CPU',
                     cpu_or_gpu_value='AVX2',
                     extra_tokens='',
                     model='GCE',
                     os='Debian9',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     test_filter='All',
                     is_testing_exceptions='True')
  )

  builder = 'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-All-Android'
  yield (
      api.test('failed_infra_step') +
      api.properties(arch='x86',
                     buildername=builder,
                     compiler='Clang',
                     configuration='Debug',
                     cpu_or_gpu='GPU',
                     cpu_or_gpu_value='PowerVR',
                     extra_tokens='Android',
                     model='NexusPlayer',
                     os='Android',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     test_filter='All') +
      api.step_data('get swarming bot id',
                    stdout=api.raw_io.output('build123-m2--device5')) +
      api.step_data('dump log', retcode=1)
  )

  builder = 'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-All-Android'
  yield (
      api.test('failed_read_version') +
      api.properties(arch='x86',
                     buildername=builder,
                     compiler='Clang',
                     configuration='Debug',
                     cpu_or_gpu='GPU',
                     cpu_or_gpu_value='PowerVR',
                     extra_tokens='Android',
                     model='NexusPlayer',
                     os='Android',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     test_filter='All') +
      api.step_data('read /sdcard/revenge_of_the_skiabot/SK_IMAGE_VERSION',
                    retcode=1)
  )

  builder = 'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-All-Android'
  yield (
      api.test('retry_adb_command') +
      api.properties(arch='x86',
                     buildername=builder,
                     compiler='Clang',
                     configuration='Debug',
                     cpu_or_gpu='GPU',
                     cpu_or_gpu_value='PowerVR',
                     extra_tokens='Android',
                     model='NexusPlayer',
                     os='Android',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     test_filter='All') +
      api.step_data('mkdir /sdcard/revenge_of_the_skiabot/resources',
                    retcode=1)
  )

  builder = 'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Debug-All-Android'
  fail_step_name = 'mkdir /sdcard/revenge_of_the_skiabot/resources'
  yield (
      api.test('retry_adb_command_retries_exhausted') +
      api.properties(arch='x86',
                     buildername=builder,
                     compiler='Clang',
                     configuration='Debug',
                     cpu_or_gpu='GPU',
                     cpu_or_gpu_value='PowerVR',
                     extra_tokens='Android',
                     model='NexusPlayer',
                     os='Android',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     test_filter='All') +
      api.step_data('get swarming bot id',
                    stdout=api.raw_io.output('build123-m2--device5')) +
      api.step_data(fail_step_name, retcode=1) +
      api.step_data(fail_step_name + ' (attempt 2)', retcode=1) +
      api.step_data(fail_step_name + ' (attempt 3)', retcode=1)
  )

  builder = 'Test-iOS-Clang-iPhone7-GPU-GT7600-arm64-Release-All'
  fail_step_name = 'install_dm'
  yield (
      api.test('retry_ios_install') +
      api.properties(arch='arm64',
                     buildername=builder,
                     compiler='Clang',
                     configuration='Release',
                     cpu_or_gpu='GPU',
                     cpu_or_gpu_value='GT7600',
                     extra_tokens='',
                     model='iPhone7',
                     os='iOS',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     test_filter='All') +
      api.step_data(fail_step_name, retcode=1)
  )

  yield (
      api.test('retry_ios_install_retries_exhausted') +
      api.properties(arch='arm64',
                     buildername=builder,
                     compiler='Clang',
                     configuration='Release',
                     cpu_or_gpu='GPU',
                     cpu_or_gpu_value='GT7600',
                     extra_tokens='',
                     model='iPhone7',
                     os='iOS',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     test_filter='All') +
      api.step_data(fail_step_name, retcode=1) +
      api.step_data(fail_step_name + ' (attempt 2)', retcode=1)
  )

  builder = ('Perf-Android-Clang-NexusPlayer-CPU-Moorefield-x86-Debug-All-' +
             'Android')
  yield (
    api.test('cpu_scale_failed_once') +
    api.properties(arch='x86',
                   buildername=builder,
                   compiler='Clang',
                   configuration='Debug',
                   cpu_or_gpu='CPU',
                   cpu_or_gpu_value='Moorefield',
                   extra_tokens='Android',
                   model='NexusPlayer',
                   os='Android',
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   test_filter='All') +
    api.step_data('Scale CPU 0 to 0.600000', retcode=1)
  )

  yield (
    api.test('cpu_scale_failed') +
    api.properties(arch='x86',
                   buildername=builder,
                   compiler='Clang',
                   configuration='Debug',
                   cpu_or_gpu='CPU',
                   cpu_or_gpu_value='Moorefield',
                   extra_tokens='Android',
                   model='NexusPlayer',
                   os='Android',
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   test_filter='All') +
    api.step_data('get swarming bot id',
                  stdout=api.raw_io.output('skia-rpi-022')) +
    api.step_data('Scale CPU 0 to 0.600000', retcode=1)+
    api.step_data('Scale CPU 0 to 0.600000 (attempt 2)', retcode=1)+
    api.step_data('Scale CPU 0 to 0.600000 (attempt 3)', retcode=1)
  )

  builder = ('Perf-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Release'
             '-All-Android')
  yield (
    api.test('cpu_scale_failed_golo') +
    api.properties(arch='arm64',
                   buildername=builder,
                   compiler='Clang',
                   configuration='Release',
                   cpu_or_gpu='GPU',
                   cpu_or_gpu_value='Adreno418',
                   extra_tokens='Android',
                   model='Nexus5x',
                   os='Android',
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   test_filter='All') +
    api.step_data('get swarming bot id',
                  stdout=api.raw_io.output('build123-m2--device5')) +
    api.step_data('Scale CPU 4 to 0.600000', retcode=1)+
    api.step_data('Scale CPU 4 to 0.600000 (attempt 2)', retcode=1)+
    api.step_data('Scale CPU 4 to 0.600000 (attempt 3)', retcode=1)
  )
