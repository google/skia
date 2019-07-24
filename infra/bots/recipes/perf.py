# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming perf.


import calendar
import os


DEPS = [
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


def upload_perf_results(buildername):
  if 'Release' not in buildername:
    return False
  skip_upload_bots = [
    'ASAN',
    'Coverage',
    'MSAN',
    'TSAN',
    'UBSAN',
    'Valgrind',
  ]
  for s in skip_upload_bots:
    if s in buildername:
      return False
  return True


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

    if 'BonusConfigs' in bot or ('SAN' in bot and 'GCE' in bot):
      configs += [
          'f16',
          'srgb',
          'esrgb',
          'narrow',
          'enarrow',
      ]

    if 'Nexus7' in bot:
      args.append('--purgeBetweenBenches')  # Debugging skia:8929

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
      # iOS crashes with MSAA (skia:6399)
      # Nexus7 (Tegra3) does not support MSAA.
      if ('iOS'         in bot or
          'Nexus7'      in bot):
        sample_count = ''
    elif 'Intel' in bot:
      # We don't want to test MSAA on older Intel chipsets. These are all newer (Gen9).
      if 'Iris655' not in bot and 'Iris640' not in bot and 'Iris540' not in bot:
        sample_count = ''
    elif 'ChromeOS' in bot:
      gl_prefix = 'gles'

    configs.extend([gl_prefix, gl_prefix + 'srgb'])
    if sample_count:
      configs.append(gl_prefix + 'msaa' + sample_count)

    # We want to test both the OpenGL config and the GLES config on Linux Intel:
    # GL is used by Chrome, GLES is used by ChromeOS.
    if 'Intel' in bot and api.vars.is_linux:
      configs.extend(['gles', 'glessrgb'])

    if 'CommandBuffer' in bot:
      configs = ['commandbuffer']

    if 'Vulkan' in bot:
      configs = ['vk']
      if 'Android' in bot:
        configs.append('vkmsaa4')
      else:
        # skbug.com/9023
        if 'IntelHD405' not in bot:
          configs.append('vkmsaa8')

    if 'Metal' in bot:
      configs = ['mtl']
      if 'iOS' in bot:
        configs.append('mtlmsaa4')
      else:
        configs.append('mtlmsaa8')

    if 'ANGLE' in bot:
      # Test only ANGLE configs.
      configs = ['angle_d3d11_es2']
      if sample_count:
        configs.append('angle_d3d11_es2_msaa' + sample_count)
      if 'QuadroP400' in bot:
        # See skia:7823 and chromium:693090.
        configs.append('angle_gl_es2')
        if sample_count:
          configs.append('angle_gl_es2_msaa' + sample_count)

    if 'ChromeOS' in bot:
      # Just run GLES for now - maybe add gles_msaa4 in the future
      configs = ['gles']

  args.append('--config')
  args.extend(configs)

  # By default, we test with GPU threading enabled, unless specifically
  # disabled.
  if 'NoGPUThreads' in bot:
    args.extend(['--gpuThreads', '0'])

  if 'Debug' in bot or 'ASAN' in bot or 'Valgrind' in bot:
    args.extend(['--loops',   '1'])
    args.extend(['--samples', '1'])
    # Ensure that the bot framework does not think we have timed out.
    args.extend(['--keepAlive', 'true'])

  # skia:9036
  if 'NVIDIA_Shield' in bot or 'Chorizo' in bot:
    args.extend(['--dontReduceOpListSplitting'])

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
  if 'MoltenVK' in bot:
    # skbug.com/7962
    match.append('~^path_text_clipped_uncached$')
    match.append('~^path_text_uncached$')
  if ('Intel' in bot and api.vars.is_linux and not 'Vulkan' in bot):
    # TODO(dogben): Track down what's causing bots to die.
    verbose = True
  if 'IntelHD405' in bot and api.vars.is_linux and 'Vulkan' in bot:
    # skia:7322
    match.append('~desk_carsvg.skp_1')
    match.append('~desk_googlehome.skp')
    match.append('~desk_tiger8svg.skp_1')
    match.append('~desk_wowwiki.skp')
    match.append('~desk_ynevsvg.skp_1.1')
    match.append('~desk_nostroke_tiger8svg.skp')
    match.append('~keymobi_booking_com.skp_1')
    match.append('~keymobi_booking_com.skp_1_mpd')
    match.append('~keymobi_cnn_article.skp_1')
    match.append('~keymobi_cnn_article.skp_1_mpd')
    match.append('~keymobi_forecast_io.skp_1')
    match.append('~keymobi_forecast_io.skp_1_mpd')
    match.append('~keymobi_sfgate.skp_1')
    match.append('~keymobi_techcrunch_com.skp_1.1')
    match.append('~keymobi_techcrunch.skp_1.1')
    match.append('~keymobi_techcrunch.skp_1.1_mpd')
    match.append('~svgparse_Seal_of_California.svg_1.1')
    match.append('~svgparse_NewYork-StateSeal.svg_1.1')
    match.append('~svgparse_Vermont_state_seal.svg_1')
    match.append('~tabl_gamedeksiam.skp_1.1')
    match.append('~tabl_pravda.skp_1')
    match.append('~top25desk_ebay_com.skp_1.1')
    match.append('~top25desk_ebay.skp_1.1')
    match.append('~top25desk_ebay.skp_1.1_mpd')
  if 'Vulkan' in bot and ('Nexus5x' in bot or 'GTX660' in bot):
    # skia:8523 skia:9271
    match.append('~compositing_images')
  if 'MacBook10.1' in bot and 'CommandBuffer' in bot:
    match.append('~^desk_micrographygirlsvg.skp_1.1$')
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
  b = api.properties['buildername']
  if upload_perf_results(b):
    api.flavor.create_clean_device_dir(
        api.flavor.device_dirs.perf_data_dir)

  # Run nanobench.
  properties = [
    '--properties',
    'gitHash', api.properties['revision'],
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
      '--dontReduceOpListSplitting',
      '--match', # skia:6687
      '~matrixconvolution',
      '~blur_image_filter',
      '~blur_0.01',
      '~GM_animated-image-blurs',
      '~blendmode_mask_',
      '~desk_carsvg.skp',
      '~^path_text_clipped', # Bot times out; skia:7190
      '~shapes_rrect_inner_rrect_50_500x500', # skia:7551
      '~compositing_images',
    ])
    if 'Debug' in api.vars.builder_name:
      args.extend(['--loops', '1'])

  if upload_perf_results(b):
    now = api.time.utcnow()
    ts = int(calendar.timegm(now.utctimetuple()))
    json_path = api.flavor.device_path_join(
        api.flavor.device_dirs.perf_data_dir,
        'nanobench_%s_%d.json' % (api.properties['revision'], ts))
    args.extend(['--outResultsFile', json_path])
    args.extend(properties)

    keys_blacklist = ['configuration', 'role', 'test_filter']
    args.append('--key')
    for k in sorted(api.vars.builder_cfg.keys()):
      if not k in keys_blacklist:
        args.extend([k, api.vars.builder_cfg[k]])

  api.run(api.flavor.step, target, cmd=args,
          abort_on_failure=False)

  # Copy results to swarming out dir.
  if upload_perf_results(b):
    api.file.ensure_directory(
        'makedirs perf_dir',
        api.flavor.host_dirs.perf_data_dir)
    api.flavor.copy_directory_contents_to_host(
        api.flavor.device_dirs.perf_data_dir,
        api.flavor.host_dirs.perf_data_dir)


def RunSteps(api):
  api.vars.setup()
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)
  api.flavor.setup()

  env = {}
  if 'iOS' in api.vars.builder_name:
    env['IOS_BUNDLE_ID'] = 'com.google.nanobench'
    env['IOS_MOUNT_POINT'] = api.vars.slave_dir.join('mnt_iosdevice')
  with api.env(env):
    try:
      if 'Chromecast' in api.vars.builder_name:
        api.flavor.install(resources=True, skps=True)
      else:
        api.flavor.install(skps=True, images=True, svgs=True, resources=True)
      perf_steps(api)
    finally:
      api.flavor.cleanup_steps()
    api.run.check_failure()


TEST_BUILDERS = [
  'Perf-Android-Clang-Nexus7-CPU-Tegra3-arm-Debug-All-Android',
  'Perf-Android-Clang-Nexus5-GPU-Adreno330-arm-Debug-All-Android',
  ('Perf-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Release-All-'
   'Android_NoGPUThreads'),
  'Perf-Android-Clang-Nexus5x-GPU-Adreno418-arm64-Release-All-Android_Vulkan',
  'Perf-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Release-All-Android',
  'Perf-Android-Clang-P30-GPU-MaliG76-arm64-Release-All-Android_Vulkan',
  'Perf-ChromeOS-Clang-ASUSChromebookFlipC100-GPU-MaliT764-arm-Release-All',
  'Perf-Chromecast-Clang-Chorizo-CPU-Cortex_A7-arm-Debug-All',
  'Perf-Chromecast-Clang-Chorizo-GPU-Cortex_A7-arm-Release-All',
  'Perf-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All',
  'Perf-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-ASAN',
  'Perf-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All-BonusConfigs',
  'Perf-Debian9-Clang-NUC5PPYH-GPU-IntelHD405-x86_64-Debug-All-Vulkan',
  'Perf-Debian9-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Release-All',
  ('Perf-Mac10.13-Clang-MacBook10.1-GPU-IntelHD615-x86_64-Release-All-'
   'CommandBuffer'),
  ('Perf-Mac10.13-Clang-MacBookPro11.5-GPU-RadeonHD8870M-x86_64-Release-All-'
   'Metal'),
  ('Perf-Mac10.13-Clang-MacBookPro11.5-GPU-RadeonHD8870M-x86_64-Release-All-'
   'MoltenVK_Vulkan'),
  ('Perf-Mac10.13-Clang-MacMini7.1-GPU-IntelIris5100-x86_64-Release-All-'
   'CommandBuffer'),
  ('Perf-Ubuntu17-GCC-Golo-GPU-QuadroP400-x86_64-Release-All-'
    'Valgrind_SK_CPU_LIMIT_SSE41'),
  'Perf-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-ANGLE',
  'Perf-iOS-Clang-iPadPro-GPU-PowerVRGT7800-arm64-Release-All',
  'Perf-iOS-Clang-iPhone6-GPU-PowerVRGX6450-arm64-Release-All-Metal',
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
