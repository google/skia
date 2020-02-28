# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming test.


import imp
import os


DEPS = [
  'env',
  'flavor',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'run',
  'vars',
]


def upload_dm_results(buildername):
  skip_upload_bots = [
    'ASAN',
    'Coverage',
    'MSAN',
    'TSAN',
    'Valgrind',
  ]
  for s in skip_upload_bots:
    if s in buildername:
      return False
  return True


def test_steps(api):
  """Run the DM test."""
  b = api.properties['buildername']
  use_hash_file = False
  if upload_dm_results(b):
    host_dm_dir = str(api.flavor.host_dirs.dm_dir)
    api.flavor.create_clean_host_dir(api.path['start_dir'].join('test'))
    device_dm_dir = str(api.flavor.device_dirs.dm_dir)
    if host_dm_dir != device_dm_dir:
      api.flavor.create_clean_device_dir(device_dm_dir)

    # Obtain the list of already-generated hashes.
    hash_filename = 'uninteresting_hashes.txt'

    host_hashes_file = api.vars.tmp_dir.join(hash_filename)
    hashes_file = api.flavor.device_path_join(
        api.flavor.device_dirs.tmp_dir, hash_filename)
    api.run(
        api.python.inline,
        'get uninteresting hashes',
        program="""
        import contextlib
        import math
        import socket
        import sys
        import time
        import urllib2

        HASHES_URL = sys.argv[1]
        RETRIES = 5
        TIMEOUT = 60
        WAIT_BASE = 15

        socket.setdefaulttimeout(TIMEOUT)
        for retry in range(RETRIES):
          try:
            with contextlib.closing(
                urllib2.urlopen(HASHES_URL, timeout=TIMEOUT)) as w:
              hashes = w.read()
              with open(sys.argv[2], 'w') as f:
                f.write(hashes)
                break
          except Exception as e:
            print 'Failed to get uninteresting hashes from %s:' % HASHES_URL
            print e
            if retry == RETRIES:
              raise
            waittime = WAIT_BASE * math.pow(2, retry)
            print 'Retry in %d seconds.' % waittime
            time.sleep(waittime)
        """,
        args=[api.properties['gold_hashes_url'], host_hashes_file],
        abort_on_failure=False,
        fail_build_on_failure=False,
        infra_step=True)

    if api.path.exists(host_hashes_file):
      api.flavor.copy_file_to_device(host_hashes_file, hashes_file)
      use_hash_file = True

  # Find DM flags.
  dm_flags_path = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                               os.pardir, 'recipe_modules', 'vars', 'resources',
                               'dm_flags.py')
  dm_flags = imp.load_source('dm_flags', dm_flags_path)
  issue = str(api.vars.issue) if api.vars.is_trybot else ''
  patchset = str(api.vars.patchset) if api.vars.is_trybot else ''
  patch_storage = str(api.vars.patch_storage) if api.vars.is_trybot else ''
  args = dm_flags.dm_flags(
    bot=api.vars.builder_name,
    parts=api.vars.builder_cfg,
    task_id=api.properties['task_id'],
    revision=api.properties['revision'],
    issue=str(api.vars.issue) if api.vars.is_trybot else '',
    patchset=str(api.vars.patchset) if api.vars.is_trybot else '',
    patch_storage=str(api.vars.patch_storage) if api.vars.is_trybot else '',
    buildbucket_build_id=api.properties.get('buildbucket_build_id', ''),
    swarming_bot_id=api.vars.swarming_bot_id,
    swarming_task_id=api.vars.swarming_task_id,
    internal_hardware_label=api.vars.internal_hardware_label,
  )

  # Paths to required resources.
  args.extend([
    '--resourcePath', api.flavor.device_dirs.resource_dir,
    '--skps', api.flavor.device_dirs.skp_dir,
    '--images', api.flavor.device_path_join(
        api.flavor.device_dirs.images_dir, 'dm'),
    '--colorImages', api.flavor.device_path_join(
        api.flavor.device_dirs.images_dir, 'colorspace'),
    '--svgs', api.flavor.device_dirs.svg_dir,
  ])
  if 'Lottie' in api.vars.builder_cfg.get('extra_config', ''):
    args.extend([
      '--lotties',
      api.flavor.device_path_join(
          api.flavor.device_dirs.resource_dir, 'skottie'),
      api.flavor.device_dirs.lotties_dir,
    ])

  if use_hash_file:
    args.extend(['--uninterestingHashesFile', hashes_file])
  if upload_dm_results(b):
    args.extend(['--writePath', api.flavor.device_dirs.dm_dir])

  # Run DM.
  api.run(api.flavor.step, 'dm', cmd=args, abort_on_failure=False)

  if upload_dm_results(b):
    # Copy images and JSON to host machine if needed.
    api.flavor.copy_directory_contents_to_host(
        api.flavor.device_dirs.dm_dir, api.flavor.host_dirs.dm_dir)


def RunSteps(api):
  api.vars.setup()
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)
  api.flavor.setup('dm')

  try:
    if 'Lottie' in api.vars.builder_name:
      api.flavor.install(resources=True, lotties=True)
    else:
      api.flavor.install(skps=True, images=True, svgs=True, resources=True)
    test_steps(api)
  finally:
    api.flavor.cleanup_steps()
  api.run.check_failure()


TEST_BUILDERS = [
  'Test-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-Android',
  'Test-Android-Clang-GalaxyS6-GPU-MaliT760-arm64-Debug-All-Android',
  ('Test-Android-Clang-GalaxyS6-GPU-MaliT760-arm64-Debug-All'
   '-Android_NoGPUThreads'),
  ('Test-Android-Clang-GalaxyS7_G930FD-GPU-MaliT880-arm64-Release-All'
   '-Android_Vulkan'),
  'Test-Android-Clang-MotoG4-CPU-Snapdragon617-arm-Release-All-Android',
  'Test-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Debug-All-Android_CCPR',
  'Test-Android-Clang-Nexus5-GPU-Adreno330-arm-Release-All-Android',
  'Test-Android-Clang-Nexus7-CPU-Tegra3-arm-Release-All-Android',
  'Test-Android-Clang-Pixel-GPU-Adreno530-arm64-Debug-All-Android_Vulkan',
  'Test-Android-Clang-Pixel-GPU-Adreno530-arm-Debug-All-Android_ASAN',
  'Test-Android-Clang-Pixel2XL-GPU-Adreno540-arm64-Debug-All-Android',
  'Test-Android-Clang-Pixel3-GPU-Adreno630-arm64-Debug-All-Android_Vulkan',
  'Test-Android-Clang-Pixel3a-GPU-Adreno615-arm64-Debug-All-Android',
  ('Test-ChromeOS-Clang-AcerChromebookR13Convertible-GPU-PowerVRGX6250-'
   'arm-Debug-All'),
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-ASAN',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-BonusConfigs',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-shard_00_10-Coverage',
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-MSAN',
  ('Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All'
   '-SK_USE_DISCARDABLE_SCALEDIMAGECACHE'),
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All-Lottie',
  ('Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All'
   '-SK_FORCE_RASTER_PIPELINE_BLITTER'),
  'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All-TSAN',
  'Test-Debian9-Clang-GCE-GPU-SwiftShader-x86_64-Release-All-SwiftShader',
  'Test-Debian9-Clang-NUC5PPYH-GPU-IntelHD405-x86_64-Release-All-Vulkan',
  'Test-Debian9-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All-Vulkan',
  'Test-Debian10-GCC-GCE-CPU-AVX2-x86_64-Debug-All-Docker',
  'Test-iOS-Clang-iPhone6-GPU-PowerVRGX6450-arm64-Release-All-Metal',
  ('Test-Mac10.13-Clang-MacBook10.1-GPU-IntelHD615-x86_64-Release-All'
   '-NativeFonts'),
  'Test-Mac10.13-Clang-MacBookPro11.5-CPU-AVX2-x86_64-Debug-All-PDF',
  'Test-Mac10.13-Clang-MacBookPro11.5-CPU-AVX2-x86_64-Release-All',
  'Test-Mac10.13-Clang-MacBookPro11.5-GPU-RadeonHD8870M-x86_64-Debug-All-Metal',
  ('Test-Mac10.13-Clang-MacMini7.1-GPU-IntelIris5100-x86_64-Debug-All'
   '-CommandBuffer'),
  'Test-Mac10.14-Clang-MacBookAir7.2-GPU-IntelHD6000-x86_64-Debug-All',
  'Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-Vulkan',
  ('Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All'
   '-Valgrind_AbandonGpuContext_SK_CPU_LIMIT_SSE41'),
  ('Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All'
   '-Valgrind_PreAbandonGpuContext_SK_CPU_LIMIT_SSE41'),
  'Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-DDL1',
  'Test-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-DDL3',
  'Test-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-BonusConfigs',
  'Test-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-GpuTess',
  'Test-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Debug-All-NonNVPR',
  ('Test-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All'
   '-ReleaseAndAbandonGpuContext'),
  'Test-Win10-Clang-NUC5i7RYH-CPU-AVX2-x86_64-Debug-All-NativeFonts_GDI',
  'Test-Win10-Clang-NUC5i7RYH-GPU-IntelIris6100-x86_64-Release-All-ANGLE',
  'Test-Win10-Clang-NUCD34010WYKH-GPU-IntelHD4400-x86_64-Release-All-ANGLE',
  'Test-Win10-Clang-ShuttleA-GPU-GTX660-x86_64-Release-All-Vulkan',
  'Test-Win10-Clang-ShuttleA-GPU-RadeonHD7770-x86_64-Release-All-Vulkan',
  'Test-Win10-Clang-ShuttleC-GPU-GTX960-x86_64-Debug-All-ANGLE',
  'Test-Win10-MSVC-LenovoYogaC630-GPU-Adreno630-arm64-Debug-All-ANGLE',
  'Test-Win2019-Clang-GCE-CPU-AVX2-x86_64-Debug-All-FAAA',
  'Test-Win2019-Clang-GCE-CPU-AVX2-x86_64-Debug-All-FSAA',
  'Test-iOS-Clang-iPadPro-GPU-PowerVRGT7800-arm64-Release-All',
  'Test-Mac10.13-Clang-MacBook10.1-GPU-IntelHD615-x86_64-Debug-All-CommandBuffer',
  'Test-Android-Clang-TecnoSpark3Pro-GPU-PowerVRGE8320-arm-Debug-All-Android',
]


def GenTests(api):
  for builder in TEST_BUILDERS:
    test = (
      api.test(builder) +
      api.properties(buildername=builder,
                     buildbucket_build_id='123454321',
                     revision='abc123',
                     path_config='kitchen',
                     gold_hashes_url='https://example.com/hashes.txt',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     task_id='task_12345') +
      api.path.exists(
          api.path['start_dir'].join('skia'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      ) +
      api.step_data('get swarming bot id',
          stdout=api.raw_io.output('skia-bot-123')) +
      api.step_data('get swarming task id',
          stdout=api.raw_io.output('123456'))
    )
    if 'Win' in builder and not 'LenovoYogaC630' in builder:
      test += api.platform('win', 64)

    yield test

  builder = 'Test-Win8-Clang-Golo-CPU-AVX-x86-Debug-All'
  yield (
    api.test('trybot') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   gold_hashes_url='https://example.com/hashes.txt',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   task_id='task_12345') +
    api.platform('win', 64) +
    api.properties(patch_storage='gerrit') +
    api.properties.tryserver(
          buildername=builder,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )+
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    )
  )

  builder = 'Test-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All'
  yield (
    api.test('failed_dm') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   gold_hashes_url='https://example.com/hashes.txt',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   task_id='task_12345') +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    api.step_data('symbolized dm', retcode=1)
  )

  builder = 'Test-Android-Clang-Nexus7-GPU-Tegra3-arm-Release-All-Android'
  yield (
    api.test('failed_get_hashes') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   gold_hashes_url='https://example.com/hashes.txt',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   task_id='task_12345') +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    api.step_data('get uninteresting hashes', retcode=1)
  )

  builder = 'Test-Android-Clang-Nexus7-GPU-Tegra3-arm-Debug-All-Android'
  retry_step_name = ('push [START_DIR]/skia/resources/* '
                     '/sdcard/revenge_of_the_skiabot/resources.push '
                     '[START_DIR]/skia/resources/file1')
  yield (
    api.test('failed_push') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   gold_hashes_url='https://example.com/hashes.txt',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   task_id='task_12345') +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    api.step_data('get swarming bot id',
                  stdout=api.raw_io.output('build123-m2--device5')) +
    api.step_data(retry_step_name, retcode=1) +
    api.step_data(retry_step_name + ' (attempt 2)', retcode=1) +
    api.step_data(retry_step_name + ' (attempt 3)', retcode=1)
  )

  retry_step_name = 'adb pull.pull /sdcard/revenge_of_the_skiabot/dm_out'
  yield (
    api.test('failed_pull') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   gold_hashes_url='https://example.com/hashes.txt',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   task_id='task_12345') +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    api.step_data('dm', retcode=1) +
    api.step_data(retry_step_name, retcode=1) +
    api.step_data(retry_step_name + ' (attempt 2)', retcode=1) +
    api.step_data(retry_step_name + ' (attempt 3)', retcode=1)
  )

  yield (
    api.test('internal_bot_5') +
    api.properties(buildername=builder,
                   buildbucket_build_id='123454321',
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   gold_hashes_url='https://example.com/hashes.txt',
                   internal_hardware_label='5',
                   task_id='task_12345') +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    )
  )
