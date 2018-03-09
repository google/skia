# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming perf.


import calendar
import os


DEPS = [
  'core',
  'env',
  'flavor',
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'recipe_engine/time',
  'run',
  'vars',
]


def nanobench_flags(api, bot):
  args = ['--pre_log']

  if 'GPU' in bot:
    args.append('--images')
    args.extend(['--gpuStatsDump', 'true'])

  args.extend(['--scales', '1.0', '1.1'])

  if 'iOS' in bot:
    args.extend(['--skps', 'ignore_skps'])

  configs = []
  if api.vars.builder_cfg.get('cpu_or_gpu') == 'CPU':
    args.append('--nogpu')
    configs.extend(['8888', 'nonrendering'])

    if '-arm-' not in bot:
      # For Android CPU tests, these take too long and cause the task to time
      # out.
      configs += [ 'f16', 'srgb' ]
    if '-GCE-' in bot:
      configs += [ '565' ]

  elif api.vars.builder_cfg.get('cpu_or_gpu') == 'GPU':
    args.append('--nocpu')

    gl_prefix = 'gl'
    sample_count = '8'
    if 'Android' in bot or 'iOS' in bot:
      sample_count = '4'
      # The NVIDIA_Shield has a regular OpenGL implementation. We bench that
      # instead of ES.
      if 'NVIDIA_Shield' not in bot:
        gl_prefix = 'gles'
      # The NP produces a long error stream when we run with MSAA.
      # iOS crashes (skia:6399)
      # Nexus7 (Tegra3) does not support MSAA.
      if ('NexusPlayer' in bot or
          'iOS'         in bot or
          'Nexus7'      in bot):
        sample_count = ''
    elif 'Intel' in bot:
      sample_count = ''
    elif 'ChromeOS' in bot:
      gl_prefix = 'gles'

    configs.extend([gl_prefix, gl_prefix + 'srgb'])
    if sample_count is not '':
      configs.append(gl_prefix + 'msaa' + sample_count)
      if ('TegraX1' in bot or
          'Quadro' in bot or
          'GTX' in bot or
          ('GT610' in bot and 'Ubuntu17' not in bot)):
        configs.extend([gl_prefix + 'nvpr' + sample_count])

    # We want to test both the OpenGL config and the GLES config on Linux Intel:
    # GL is used by Chrome, GLES is used by ChromeOS.
    if 'Intel' in bot and api.vars.is_linux:
      configs.extend(['gles', 'glessrgb'])

    # The following devices do not support glessrgb.
    if 'glessrgb' in configs:
      if ('IntelHD405'    in bot or
          'IntelIris640'  in bot or
          'IntelBayTrail' in bot or
          'IntelHD2000'   in bot or
          'AndroidOne'    in bot or
          'Nexus7'        in bot or
          'NexusPlayer'   in bot):
        configs.remove('glessrgb')

    if 'CommandBuffer' in bot:
      configs = ['commandbuffer']
    if 'Vulkan' in bot:
      configs = ['vk']

    if 'ANGLE' in bot:
      # Test only ANGLE configs.
      configs = ['angle_d3d11_es2']
      if sample_count is not '':
        configs.append('angle_d3d11_es2_msaa' + sample_count)

    if 'ChromeOS' in bot:
      # Just run GLES for now - maybe add gles_msaa4 in the future
      configs = ['gles']

  args.append('--config')
  args.extend(configs)

  # By default, we test with GPU threading enabled. Leave PixelC devices
  # running without threads, just to get some coverage of that code path.
  if 'PixelC' in bot:
    args.extend(['--gpuThreads', '0'])

  if 'Valgrind' in bot:
    # Don't care about Valgrind performance.
    args.extend(['--loops',   '1'])
    args.extend(['--samples', '1'])
    # Ensure that the bot framework does not think we have timed out.
    args.extend(['--keepAlive', 'true'])

  # Some people don't like verbose output.
  verbose = False

  match = []
  if 'Android' in bot:
    # Segfaults when run as GPU bench. Very large texture?
    match.append('~blurroundrect')
    match.append('~patch_grid')  # skia:2847
    match.append('~desk_carsvg')
  if 'Nexus5' in bot:
    match.append('~keymobi_shop_mobileweb_ebay_com.skp')  # skia:5178
  if 'iOS' in bot:
    match.append('~blurroundrect')
    match.append('~patch_grid')  # skia:2847
    match.append('~desk_carsvg')
    match.append('~keymobi')
    match.append('~path_hairline')
    match.append('~GLInstancedArraysBench') # skia:4714
  if 'IntelIris540' in bot and 'ANGLE' in bot:
    match.append('~tile_image_filter_tiled_64')  # skia:6082
  if 'Vulkan' in bot and 'IntelIris540' in bot and 'Win' in bot:
    # skia:6398
    match.append('~GM_varied_text_clipped_lcd')
    match.append('~GM_varied_text_ignorable_clip_lcd')
    match.append('~blendmode_mask_DstATop')
    match.append('~blendmode_mask_SrcIn')
    match.append('~blendmode_mask_SrcOut')
    match.append('~blendmode_mask_Src')
    match.append('~fontscaler_lcd')
    match.append('~rotated_rects_aa_alternating_transparent_and_opaque_src')
    match.append('~rotated_rects_aa_changing_transparent_src')
    match.append('~rotated_rects_aa_same_transparent_src')
    match.append('~shadermask_LCD_FF')
    match.append('~srcmode_rects_1')
    match.append('~text_16_LCD_88')
    match.append('~text_16_LCD_BK')
    match.append('~text_16_LCD_FF')
    match.append('~text_16_LCD_WT')
    # skia:6863
    match.append('~desk_skbug6850overlay2')
    match.append('~desk_googlespreadsheet')
    match.append('~desk_carsvg')
  if ('Vulkan' in bot and ('RadeonR9M470X' in bot or 'RadeonHD7770' in bot) and
      'Win' in bot):
    # skia:7677
    match.append('~path_text_clipped_uncached')
  if ('Intel' in bot and api.vars.is_linux and not 'Vulkan' in bot):
    # TODO(dogben): Track down what's causing bots to die.
    verbose = True
  if 'IntelHD405' in bot and api.vars.is_linux and 'Vulkan' in bot:
    # skia:7322
    match.append('~desk_tiger8svg.skp_1')
    match.append('~keymobi_techcrunch_com.skp_1.1')
    match.append('~tabl_gamedeksiam.skp_1.1')
    match.append('~tabl_pravda.skp_1')
    match.append('~top25desk_ebay_com.skp_1.1')
  if 'Vulkan' in bot and 'NexusPlayer' in bot:
    match.append('~blendmode_') # skia:6691
  if ('ASAN' in bot or 'UBSAN' in bot) and 'CPU' in bot:
    # floor2int_undef benches undefined behavior, so ASAN correctly complains.
    match.append('~^floor2int_undef$')

  # We do not need or want to benchmark the decodes of incomplete images.
  # In fact, in nanobench we assert that the full image decode succeeds.
  match.append('~inc0.gif')
  match.append('~inc1.gif')
  match.append('~incInterlaced.gif')
  match.append('~inc0.jpg')
  match.append('~incGray.jpg')
  match.append('~inc0.wbmp')
  match.append('~inc1.wbmp')
  match.append('~inc0.webp')
  match.append('~inc1.webp')
  match.append('~inc0.ico')
  match.append('~inc1.ico')
  match.append('~inc0.png')
  match.append('~inc1.png')
  match.append('~inc2.png')
  match.append('~inc12.png')
  match.append('~inc13.png')
  match.append('~inc14.png')
  match.append('~inc0.webp')
  match.append('~inc1.webp')

  if match:
    args.append('--match')
    args.extend(match)

  if verbose:
    args.append('--verbose')

  return args


def perf_steps(api):
  """Run Skia benchmarks."""
  if api.vars.upload_perf_results:
    api.flavor.create_clean_device_dir(
        api.flavor.device_dirs.perf_data_dir)

  # Run nanobench.
  properties = [
    '--properties',
    'gitHash',      api.vars.got_revision,
  ]
  if api.vars.is_trybot:
    properties.extend([
      'issue',    api.vars.issue,
      'patchset', api.vars.patchset,
      'patch_storage', api.vars.patch_storage,
    ])
  properties.extend(['swarming_bot_id', api.vars.swarming_bot_id])
  properties.extend(['swarming_task_id', api.vars.swarming_task_id])

  target = 'nanobench'
  args = [
      target,
      '-i',       api.flavor.device_dirs.resource_dir,
      '--skps',   api.flavor.device_dirs.skp_dir,
      '--images', api.flavor.device_path_join(
          api.flavor.device_dirs.images_dir, 'nanobench'),
  ]

  # Do not run svgs on Valgrind.
  if 'Valgrind' not in api.vars.builder_name:
    args.extend(['--svgs',  api.flavor.device_dirs.svg_dir])

  args.extend(nanobench_flags(api, api.vars.builder_name))

  if 'Chromecast' in api.vars.builder_cfg.get('os', ''):
    # Due to limited disk space, run a watered down perf run on Chromecast.
    args = [target]
    if api.vars.builder_cfg.get('cpu_or_gpu') == 'CPU':
      args.extend(['--nogpu', '--config', '8888'])
    elif api.vars.builder_cfg.get('cpu_or_gpu') == 'GPU':
      args.extend(['--nocpu', '--config', 'gles'])
    args.extend([
      '-i', api.flavor.device_dirs.resource_dir,
      '--images', api.flavor.device_path_join(
          api.flavor.device_dirs.resource_dir, 'images', 'color_wheel.jpg'),
      '--skps',  api.flavor.device_dirs.skp_dir,
      '--pre_log',
      '--match', # skia:6687
      '~matrixconvolution',
      '~blur_image_filter',
      '~blur_0.01',
      '~GM_animated-image-blurs',
      '~blendmode_mask_',
      '~desk_carsvg.skp',
      '~^path_text_clipped', # Bot times out; skia:7190
      '~shapes_rrect_inner_rrect_50_500x500', # skia:7551
    ])

  if api.vars.upload_perf_results:
    now = api.time.utcnow()
    ts = int(calendar.timegm(now.utctimetuple()))
    json_path = api.flavor.device_path_join(
        api.flavor.device_dirs.perf_data_dir,
        'nanobench_%s_%d.json' % (api.vars.got_revision, ts))
    args.extend(['--outResultsFile', json_path])
    args.extend(properties)

    keys_blacklist = ['configuration', 'role', 'test_filter']
    args.append('--key')
    for k in sorted(api.vars.builder_cfg.keys()):
      if not k in keys_blacklist:
        args.extend([k, api.vars.builder_cfg[k]])

  # See skia:2789.
  if 'AbandonGpuContext' in api.vars.extra_tokens:
    args.extend(['--abandonGpuContext'])

  api.run(api.flavor.step, target, cmd=args,
          abort_on_failure=False)

  # Copy results to swarming out dir.
  if api.vars.upload_perf_results:
    api.file.ensure_directory('makedirs perf_dir',
                              api.path.dirname(api.vars.perf_data_dir))
    api.flavor.copy_directory_contents_to_host(
        api.flavor.device_dirs.perf_data_dir,
        api.vars.perf_data_dir)


def RunSteps(api):
  api.core.setup()
  env = {}
  if 'iOS' in api.vars.builder_name:
    env['IOS_BUNDLE_ID'] = 'com.google.nanobench'
    env['IOS_MOUNT_POINT'] = api.vars.slave_dir.join('mnt_iosdevice')
  with api.env(env):
    try:
      if 'Chromecast' in api.vars.builder_name:
        api.flavor.install(resources=True, skps=True)
      else:
        api.flavor.install_everything()
      perf_steps(api)
    finally:
      api.flavor.cleanup_steps()
    api.run.check_failure()


TEST_BUILDERS = [
  ('Perf-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Debug-All-'
   'Android_Vulkan'),
  'Perf-Android-Clang-Nexus5-GPU-Adreno330-arm-Debug-All-Android',
  'Perf-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Release-All-Android',
  'Perf-Android-Clang-Nexus7-CPU-Tegra3-arm-Release-All-Android',
  'Perf-Android-Clang-Nexus7-GPU-Tegra3-arm-Release-All-Android',
  'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Release-All-Android',
  'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Release-All-Android_Vulkan',
  'Perf-Android-Clang-PixelC-GPU-TegraX1-arm64-Release-All-Android_Skpbench',
  'Perf-ChromeOS-Clang-ASUSChromebookFlipC100-GPU-MaliT764-arm-Release-All',
  'Perf-Chromecast-GCC-Chorizo-CPU-Cortex_A7-arm-Debug-All',
  'Perf-Chromecast-GCC-Chorizo-GPU-Cortex_A7-arm-Release-All',
  'Perf-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-ASAN',
  'Perf-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All',
  'Perf-Mac-Clang-MacMini7.1-CPU-AVX-x86_64-Release-All',
  'Perf-Mac-Clang-MacMini7.1-GPU-IntelIris5100-x86_64-Release-All',
  ('Perf-Mac-Clang-MacMini7.1-GPU-IntelIris5100-x86_64-Release-All-'
   'CommandBuffer'),
  'Perf-Ubuntu16-Clang-NUC5PPYH-GPU-IntelHD405-x86_64-Debug-All-Vulkan',
  'Perf-Ubuntu16-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All-Vulkan',
  'Perf-Ubuntu16-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Release-All',
  ('Perf-Ubuntu17-GCC-Golo-GPU-QuadroP400-x86_64-Release-All-'
   'Valgrind_AbandonGpuContext_SK_CPU_LIMIT_SSE41'),
  ('Perf-Ubuntu17-GCC-Golo-GPU-QuadroP400-x86_64-Release-All-'
   'Valgrind_SK_CPU_LIMIT_SSE41'),
  'Perf-Win10-Clang-AlphaR2-GPU-RadeonR9M470X-x86_64-Release-All-ANGLE',
  'Perf-Win10-Clang-AlphaR2-GPU-RadeonR9M470X-x86_64-Release-All-Vulkan',
  'Perf-Win10-Clang-NUC6i5SYK-GPU-IntelIris540-x86_64-Release-All-ANGLE',
  'Perf-Win10-Clang-NUC6i5SYK-GPU-IntelIris540-x86_64-Release-All-Vulkan',
  'Perf-Win10-Clang-ShuttleC-GPU-GTX960-x86_64-Release-All-ANGLE',
  'Perf-Win2016-MSVC-GCE-CPU-AVX2-x86_64-Debug-All',
  'Perf-Win2016-MSVC-GCE-CPU-AVX2-x86_64-Release-All',
  'Perf-iOS-Clang-iPadPro-GPU-GT7800-arm64-Release-All',
]


def GenTests(api):
  for builder in TEST_BUILDERS:
    test = (
      api.test(builder) +
      api.properties(buildername=builder,
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(
          api.path['start_dir'].join('skia'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      ) +
      api.step_data('get swarming bot id',
          stdout=api.raw_io.output('skia-bot-123')) +
      api.step_data('get swarming task id',
          stdout=api.raw_io.output('123456'))
    )
    if 'Win' in builder:
      test += api.platform('win', 64)

    if 'Chromecast' in builder:
      test += api.step_data(
          'read chromecast ip',
          stdout=api.raw_io.output('192.168.1.2:5555'))

    if 'ChromeOS' in builder:
      test += api.step_data(
          'read chromeos ip',
          stdout=api.raw_io.output('{"user_ip":"foo@127.0.0.1"}'))

    yield test

  builder = 'Perf-Win10-Clang-NUCD34010WYKH-GPU-IntelHD4400-x86_64-Release-All'
  yield (
    api.test('trybot') +
    api.properties(buildername=builder,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
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

  builder = ('Perf-Android-Clang-NexusPlayer-CPU-Moorefield-x86-Debug-All-' +
             'Android')
  yield (
    api.test('failed_push') +
    api.properties(buildername=builder,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
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
    api.step_data('push [START_DIR]/skia/resources/* '+
                  '/sdcard/revenge_of_the_skiabot/resources', retcode=1)
  )

  yield (
    api.test('cpu_scale_failed_once') +
    api.properties(buildername=builder,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
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
    api.step_data('Scale CPU 0 to 0.600000', retcode=1)
  )

  yield (
    api.test('cpu_scale_failed') +
    api.properties(buildername=builder,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
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
                  stdout=api.raw_io.output('skia-rpi-022')) +
    api.step_data('Scale CPU 0 to 0.600000', retcode=1)+
    api.step_data('Scale CPU 0 to 0.600000 (attempt 2)', retcode=1)+
    api.step_data('Scale CPU 0 to 0.600000 (attempt 3)', retcode=1)
  )

  builder = ('Perf-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Release'
             '-All-Android')
  yield (
    api.test('cpu_scale_failed_golo') +
    api.properties(buildername=builder,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
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
    api.step_data('Scale CPU 4 to 0.600000', retcode=1)+
    api.step_data('Scale CPU 4 to 0.600000 (attempt 2)', retcode=1)+
    api.step_data('Scale CPU 4 to 0.600000 (attempt 3)', retcode=1)
  )
