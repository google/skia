# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming perf.


import calendar

from recipe_engine import recipe_api


def nanobench_flags(bot):
  args = ['--pre_log']

  if 'GPU' in bot:
    args.append('--images')
    args.extend(['--gpuStatsDump', 'true'])

  if 'Android' in bot and 'GPU' in bot:
    args.extend(['--useThermalManager', '1,1,10,1000'])

  args.extend(['--scales', '1.0', '1.1'])

  if 'iOS' in bot:
    args.extend(['--skps', 'ignore_skps'])

  config = ['8888', 'gpu', 'nonrendering', 'hwui' ]
  if 'AndroidOne' not in bot:
    config += [ 'f16', 'srgb' ]
  if '-GCE-' in bot:
    config += [ '565' ]
  # The NP produces a long error stream when we run with MSAA.
  if 'NexusPlayer' not in bot:
    if 'Android' in bot:
      # The NVIDIA_Shield has a regular OpenGL implementation. We bench that
      # instead of ES.
      if 'NVIDIA_Shield' in bot:
        config.remove('gpu')
        config.extend(['gl', 'glmsaa4', 'glnvpr4', 'glnvprdit4'])
      else:
        config.extend(['msaa4', 'nvpr4', 'nvprdit4'])
    else:
      config.extend(['msaa16', 'nvpr16', 'nvprdit16'])

  # Bench instanced rendering on a limited number of platforms
  if 'Nexus6' in bot:
    config.append('esinst') # esinst4 isn't working yet on Adreno.
  elif 'PixelC' in bot:
    config.extend(['esinst', 'esinst4'])
  elif 'NVIDIA_Shield' in bot:
    config.extend(['glinst', 'glinst4'])
  elif 'MacMini6.2' in bot:
    config.extend(['glinst', 'glinst16'])

  if 'CommandBuffer' in bot:
    config = ['commandbuffer']
  if 'Vulkan' in bot:
    config = ['vk']

  if 'ANGLE' in bot:
    config.extend(['angle_d3d11_es2'])
    # The GL backend of ANGLE crashes on the perf bot currently.
    if 'Win' not in bot:
      config.extend(['angle_gl_es2'])

  args.append('--config')
  args.extend(config)

  if 'Valgrind' in bot:
    # Don't care about Valgrind performance.
    args.extend(['--loops',   '1'])
    args.extend(['--samples', '1'])
    # Ensure that the bot framework does not think we have timed out.
    args.extend(['--keepAlive', 'true'])

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
    'build_number', api.vars.build_number,
  ]
  if api.vars.is_trybot:
    properties.extend([
      'issue',    api.vars.issue,
      'patchset', api.vars.patchset,
      'patch_storage', api.vars.patch_storage,
    ])
  if api.vars.no_buildbot:
    properties.extend(['no_buildbot', 'True'])
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
    args.extend(['--svgs',  api.flavor.device_dirs.svg_dir])

  skip_flag = None
  if api.vars.builder_cfg.get('cpu_or_gpu') == 'CPU':
    skip_flag = '--nogpu'
  elif api.vars.builder_cfg.get('cpu_or_gpu') == 'GPU':
    skip_flag = '--nocpu'
  if skip_flag:
    args.append(skip_flag)
  args.extend(nanobench_flags(api.vars.builder_name))

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

  api.run(api.flavor.step, target, cmd=args,
          abort_on_failure=False,
          env=api.vars.default_env)

  # See skia:2789.
  if ('Valgrind' in api.vars.builder_name and
      api.vars.builder_cfg.get('cpu_or_gpu') == 'GPU'):
    abandonGpuContext = list(args)
    abandonGpuContext.extend(['--abandonGpuContext', '--nocpu'])
    api.run(api.flavor.step,
            '%s --abandonGpuContext' % target,
            cmd=abandonGpuContext, abort_on_failure=False,
            env=api.vars.default_env)

  # Copy results to swarming out dir.
  if api.vars.upload_perf_results:
    api.file.makedirs('perf_dir', api.vars.perf_data_dir)
    api.flavor.copy_directory_contents_to_host(
        api.flavor.device_dirs.perf_data_dir,
        api.vars.perf_data_dir)

class PerfApi(recipe_api.RecipeApi):
  def run(self):
    self.m.core.setup()
    try:
      self.m.flavor.install_everything()
      perf_steps(self.m)
    finally:
      self.m.flavor.cleanup_steps()
    self.m.run.check_failure()
