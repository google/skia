# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Generate flags for Nanobench.
# TODO(borenet): This is sitting in the vars recipe module as a hacky way to
# ensure that it's available in the recipe bundle while also making it easily
# usable in gen_tasks. The ultimate goal is to move this into gen_tasks itself.


import argparse
import json


def nanobench_flags(bot, parts, do_upload, revision, issue, patchset,
                    patch_storage):
  # TODO(borenet): This duplicates code in recipes_modules/vars/api.py and will
  # be removed soon.
  is_linux = (
        'Ubuntu' in bot
     or 'Debian' in bot
     or 'Housekeeper' in bot
  )
  extra_tokens = []
  if len(parts.get('extra_config', '')) > 0:
    if parts['extra_config'].startswith('SK'):
      assert parts['extra_config'].isupper()
      extra_tokens = [parts['extra_config']]
    else:
      extra_tokens = parts['extra_config'].split('_')

  args = [
      'nanobench',
      '--pre_log',
  ]

  if 'GPU' in bot:
    args.append('--images')
    args.extend(['--gpuStatsDump', 'true'])

  args.extend(['--scales', '1.0', '1.1'])

  if 'iOS' in bot:
    args.extend(['--skps', 'ignore_skps'])

  configs = []
  if parts.get('cpu_or_gpu') == 'CPU':
    args.append('--nogpu')
    configs.extend(['8888', 'nonrendering'])

    if 'BonusConfigs' in bot:
      configs = [
          'f16',
          'srgb',
          'esrgb',
          'narrow',
          'enarrow',
      ]

    if 'Nexus7' in bot:
      args.append('--purgeBetweenBenches')  # Debugging skia:8929

  elif parts.get('cpu_or_gpu') == 'GPU':
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
      # MSAA is disabled on Pixel3a (https://b.corp.google.com/issues/143074513).
      if ('iOS'     in bot or
          'Nexus7'  in bot or
          'Pixel3a' in bot):
        sample_count = ''
    elif 'Intel' in bot:
      # MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
      sample_count = ''
    elif 'ChromeOS' in bot:
      gl_prefix = 'gles'

    configs.extend([gl_prefix, gl_prefix + 'srgb'])
    if sample_count:
      configs.append(gl_prefix + 'msaa' + sample_count)

    # We want to test both the OpenGL config and the GLES config on Linux Intel:
    # GL is used by Chrome, GLES is used by ChromeOS.
    if 'Intel' in bot and is_linux:
      configs.extend(['gles', 'glessrgb'])

    if 'CommandBuffer' in bot:
      configs = ['commandbuffer']

    if 'Vulkan' in bot:
      configs = ['vk']
      if 'Android' in bot:
        # skbug.com/9274
        if 'Pixel2XL' not in bot:
          configs.append('vkmsaa4')
      else:
        # MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926, skia:9023
        if 'Intel' not in bot:
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
  if 'NVIDIA_Shield' in bot:
    args.extend(['--dontReduceOpsTaskSplitting'])

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
  if 'iOS' in bot and 'Metal' in bot:
    # skia:9799
    match.append('~compositing_images_tile_size')
  if ('Intel' in bot and is_linux and not 'Vulkan' in bot):
    # TODO(dogben): Track down what's causing bots to die.
    verbose = True
  if 'IntelHD405' in bot and is_linux and 'Vulkan' in bot:
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
  if 'Vulkan' in bot and 'GTX660' in bot:
    # skia:8523 skia:9271
    match.append('~compositing_images')
  if 'MacBook10.1' in bot and 'CommandBuffer' in bot:
    match.append('~^desk_micrographygirlsvg.skp_1.1$')
  if 'ASAN' in bot and 'CPU' in bot:
    # floor2int_undef benches undefined behavior, so ASAN correctly complains.
    match.append('~^floor2int_undef$')
  if 'AcerChromebook13_CB5_311-GPU-TegraK1' in bot:
    # skia:7551
    match.append('~^shapes_rrect_inner_rrect_50_500x500$')
  if ('Perf-Android-Clang-Pixel3a-GPU-Adreno615-arm64-Release-All-Android' in bot):
    # skia:9413
    match.append('~^path_text$')
    match.append('~^path_text_clipped_uncached$')
  if 'Perf-Android-Clang-Pixel3-GPU-Adreno630-arm64-Release-All-Android_Vulkan' in bot:
    # skia:9972
    match.append('~^path_text_clipped_uncached$')

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

  props = {
    'gitHash': revision,
    'issue': issue,
    'patchset': patchset,
    'patch_storage': patch_storage,
    'swarming_bot_id': '${SWARMING_BOT_ID}',
    'swarming_task_id': '${SWARMING_TASK_ID}',
  }

  if do_upload:
    keys_blacklist = ['configuration', 'role', 'test_filter']
    args.append('--key')
    for k in sorted(parts.keys()):
      if not k in keys_blacklist:
        args.extend([k, parts[k]])

  return args, props



if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('--bot', required=True)
  parser.add_argument('--parts', required=True)
  parser.add_argument('--do_upload', action='store_true')
  parser.add_argument('--revision', required=True)
  parser.add_argument('--issue', required=True)
  parser.add_argument('--patchset', required=True)
  parser.add_argument('--patch_storage', required=True)
  args = parser.parse_args()
  args, props = nanobench_flags(
    bot=args.bot,
    parts=json.loads(args.parts),
    do_upload=args.do_upload,
    revision=args.revision,
    issue=args.issue,
    patchset=args.patchset,
    patch_storage=args.patch_storage,
  )
  print json.dumps({
      'nanobench_flags': args,
      'nanobench_properties': props,
  }, separators=(',', ':'))
