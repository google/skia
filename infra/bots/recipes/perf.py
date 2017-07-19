# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming perf.


import calendar


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

  if 'Android' in bot and 'GPU' in bot:
    args.extend(['--useThermalManager', '1,1,10,1000'])

  args.extend(['--scales', '1.0', '1.1'])

  if 'iOS' in bot:
    args.extend(['--skps', 'ignore_skps'])

  configs = ['8888', 'nonrendering', 'hwui' ]

  if '-arm-' not in bot:
    # For Android CPU tests, these take too long and cause the task to time out.
    configs += [ 'f16', 'srgb' ]
  if '-GCE-' in bot:
    configs += [ '565' ]

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
    if 'NexusPlayer' in bot or 'iOS' in bot:
      sample_count = ''
  elif 'Intel' in bot:
    sample_count = ''
  elif 'ChromeOS' in bot:
    gl_prefix = 'gles'

  configs.append(gl_prefix)
  if sample_count is not '':
    configs.extend([gl_prefix + 'msaa' + sample_count,
      gl_prefix + 'nvpr' + sample_count,
      gl_prefix + 'nvprdit' + sample_count])

  # We want to test both the OpenGL config and the GLES config on Linux Intel:
  # GL is used by Chrome, GLES is used by ChromeOS.
  if 'Intel' in bot and api.vars.is_linux:
    configs.append('gles')

  # Bench instanced rendering on a limited number of platforms
  inst_config = gl_prefix + 'inst'
  if 'PixelC' in bot or 'NVIDIA_Shield' in bot or 'MacMini6.2' in bot:
    configs.extend([inst_config, inst_config + sample_count])

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
  if 'NexusPlayer' in bot:
    match.append('~desk_unicodetable')
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
  if ('Vulkan' in bot and ('IntelIris540' in bot or 'IntelIris640' in bot) and
      'Win' in bot):
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
  if ('Intel' in bot and api.vars.is_linux and not 'Vulkan' in bot):
    # TODO(dogben): Track down what's causing bots to die.
    verbose = True
  if 'Vulkan' in bot and 'NexusPlayer' in bot:
    match.append('~blendmode_') # skia:6691
  if 'ANGLE' in bot and 'Radeon' in bot and 'Release' in bot:
    # skia:6534
    match.append('~shapes_mixed_10000_32x33')
    match.append('~shapes_oval_10000_32x32')
    match.append('~shapes_oval_10000_32x33')
    match.append('~shapes_rect_100_500x500')
    match.append('~shapes_rrect_10000_32x32')
  if 'ANGLE' in bot and 'GTX960' in bot and 'Release' in bot:
    # skia:6534
    match.append('~shapes_mixed_10000_32x33')
    match.append('~shapes_rect_100_500x500')
    match.append('~shapes_rrect_10000_32x32')

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
      '--undefok',   # This helps branches that may not know new flags.
      '-i',       api.flavor.device_dirs.resource_dir,
      '--skps',   api.flavor.device_dirs.skp_dir,
      '--images', api.flavor.device_path_join(
          api.flavor.device_dirs.images_dir, 'nanobench'),
  ]

  # Do not run svgs on Valgrind.
  if 'Valgrind' not in api.vars.builder_name:
    if ('Vulkan' not in api.vars.builder_name or
        'NexusPlayer' not in api.vars.builder_name):
      args.extend(['--svgs',  api.flavor.device_dirs.svg_dir])

  skip_flag = None
  if api.vars.builder_cfg.get('cpu_or_gpu') == 'CPU':
    skip_flag = '--nogpu'
  elif api.vars.builder_cfg.get('cpu_or_gpu') == 'GPU':
    skip_flag = '--nocpu'
  if skip_flag:
    args.append(skip_flag)
  args.extend(nanobench_flags(api, api.vars.builder_name))

  if 'Chromecast' in api.vars.builder_cfg.get('os', ''):
    # Due to limited disk space, run a watered down perf run on Chromecast.
    args = [
      target,
      '--config',
      '8888',
      'gles',
    ]
    if api.vars.builder_cfg.get('cpu_or_gpu') == 'CPU':
      args.extend(['--nogpu'])
    elif api.vars.builder_cfg.get('cpu_or_gpu') == 'GPU':
      args.extend(['--nocpu'])
    args.extend([
      '-i', api.flavor.device_dirs.resource_dir,
      '--images', api.flavor.device_path_join(
          api.flavor.device_dirs.resource_dir, 'color_wheel.jpg'),
      '--skps',  api.flavor.device_dirs.skp_dir,
      '--pre_log',
      '--match', # skia:6581
      '~matrixconvolution',
      '~blur_image_filter',
      '~blur_0.01',
      '~GM_animated-image-blurs',
      '~blendmode_mask_',
    ])

  if api.vars.upload_perf_results:
    now = api.time.utcnow()
    ts = int(calendar.timegm(now.utctimetuple()))
    json_path = api.flavor.device_path_join(
        api.flavor.device_dirs.perf_data_dir,
        'nanobench_%s_%d.json' % (api.vars.got_revision, ts))
    args.extend(['--outResultsFile', json_path])
    args.extend(properties)

    keys_blacklist = ['configuration', 'role', 'is_trybot']
    args.append('--key')
    for k in sorted(api.vars.builder_cfg.keys()):
      if not k in keys_blacklist:
        args.extend([k, api.vars.builder_cfg[k]])

  env = {}
  if 'Ubuntu16' in api.vars.builder_name:
    # The vulkan in this asset name simply means that the graphics driver
    # supports Vulkan. It is also the driver used for GL code.
    dri_path = api.vars.slave_dir.join('linux_vulkan_intel_driver_release')
    if 'Debug' in api.vars.builder_name:
      dri_path = api.vars.slave_dir.join('linux_vulkan_intel_driver_debug')

    if 'Vulkan' in api.vars.builder_name:
      sdk_path = api.vars.slave_dir.join('linux_vulkan_sdk', 'bin')
      lib_path = api.vars.slave_dir.join('linux_vulkan_sdk', 'lib')
      env.update({
        'PATH':'%%(PATH)s:%s' % sdk_path,
        'LD_LIBRARY_PATH': '%s:%s' % (lib_path, dri_path),
        'LIBGL_DRIVERS_PATH': dri_path,
        'VK_ICD_FILENAMES':'%s' % dri_path.join('intel_icd.x86_64.json'),
      })
    else:
      # Even the non-vulkan NUC jobs could benefit from the newer drivers.
      env.update({
        'LD_LIBRARY_PATH': dri_path,
        'LIBGL_DRIVERS_PATH': dri_path,
      })

  # See skia:2789.
  extra_config_parts = api.vars.builder_cfg.get('extra_config', '').split('_')
  if 'AbandonGpuContext' in extra_config_parts:
    args.extend(['--abandonGpuContext', '--nocpu'])

  with api.env(env):
    api.run(api.flavor.step, target, cmd=args,
            abort_on_failure=False)

  # Copy results to swarming out dir.
  if api.vars.upload_perf_results:
    api.file.ensure_directory('makedirs perf_dir', api.vars.perf_data_dir)
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
  'Perf-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Debug-Android_Vulkan',
  'Perf-Android-Clang-Nexus10-CPU-Exynos5250-arm-Release-Android',
  'Perf-Android-Clang-Nexus5-GPU-Adreno330-arm-Debug-Android',
  'Perf-Android-Clang-Nexus7-GPU-Tegra3-arm-Release-Android',
  'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Release-Android',
  'Perf-Android-Clang-NexusPlayer-GPU-PowerVR-x86-Release-Android_Vulkan',
  'Perf-Android-Clang-PixelC-GPU-TegraX1-arm64-Release-Android',
  'Perf-ChromeOS-Clang-Chromebook_C100p-GPU-MaliT764-arm-Release',
  'Perf-Chromecast-GCC-Chorizo-CPU-Cortex_A7-arm-Debug',
  'Perf-Chromecast-GCC-Chorizo-GPU-Cortex_A7-arm-Release',
  'Perf-Mac-Clang-MacMini6.2-CPU-AVX-x86_64-Release',
  'Perf-Mac-Clang-MacMini6.2-GPU-IntelHD4000-x86_64-Debug-CommandBuffer',
  'Perf-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Release',
  'Perf-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-Valgrind',
  ('Perf-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-Valgrind' +
  '_AbandonGpuContext'),
  'Perf-Ubuntu16-Clang-NUC6i5SYK-GPU-IntelIris540-x86_64-Debug-Vulkan',
  'Perf-Ubuntu16-Clang-NUC6i5SYK-GPU-IntelIris540-x86_64-Release',
  'Perf-Win10-MSVC-AlphaR2-GPU-RadeonR9M470X-x86_64-Release-ANGLE',
  'Perf-Win10-MSVC-NUC6i5SYK-GPU-IntelIris540-x86_64-Release-ANGLE',
  'Perf-Win10-MSVC-NUC6i5SYK-GPU-IntelIris540-x86_64-Release-Vulkan',
  'Perf-Win10-MSVC-ShuttleC-GPU-GTX960-x86_64-Release-ANGLE',
  'Perf-Win2k8-MSVC-GCE-CPU-AVX2-x86_64-Debug',
  'Perf-Win2k8-MSVC-GCE-CPU-AVX2-x86_64-Release',
  'Perf-iOS-Clang-iPadMini4-GPU-GX6450-arm-Release'
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

  builder = 'Perf-Win10-MSVC-ShuttleB-GPU-IntelHD4600-x86_64-Release'
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

  builder = 'Perf-Android-Clang-NexusPlayer-CPU-SSE4-x86-Debug-Android'
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
