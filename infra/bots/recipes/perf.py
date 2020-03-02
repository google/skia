# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming perf.


import calendar
import imp
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
    'Valgrind',
  ]
  for s in skip_upload_bots:
    if s in buildername:
      return False
  return True


def perf_steps(api):
  """Run Skia benchmarks."""
  b = api.properties['buildername']
  if upload_perf_results(b):
    api.flavor.create_clean_device_dir(
        api.flavor.device_dirs.perf_data_dir)

  # Find nanobench flags.
  nanobench_flags_path = os.path.join(
      os.path.dirname(os.path.abspath(__file__)), os.pardir, 'recipe_modules',
      'vars', 'resources', 'nanobench_flags.py')
  nanobench_flags = imp.load_source('nanobench_flags', nanobench_flags_path)
  args, props = nanobench_flags.nanobench_flags(
    bot=b,
    parts=api.vars.builder_cfg,
    do_upload=upload_perf_results(b),
    revision=api.properties['revision'],
    issue=str(api.vars.issue) if api.vars.is_trybot else '',
    patchset=str(api.vars.patchset) if api.vars.is_trybot else '',
    patch_storage=str(api.vars.patch_storage) if api.vars.is_trybot else '',
  )
  swarming_bot_id = api.vars.swarming_bot_id
  swarming_task_id = api.vars.swarming_task_id
  if upload_perf_results(b):
    args.append('--properties')
    # Map iteration order is arbitrary; in order to maintain a consistent step
    # ordering, sort by key.
    for k in sorted(props.keys()):
      v = props[k]
      if v == '${SWARMING_BOT_ID}':
        v = swarming_bot_id
      elif v == '${SWARMING_TASK_ID}':
        v = swarming_task_id
      if v != '':
        args.extend([k, v])

  # Paths to required resources.
  args.extend([
      '-i',       api.flavor.device_dirs.resource_dir,
      '--skps',   api.flavor.device_dirs.skp_dir,
      '--images', api.flavor.device_path_join(
          api.flavor.device_dirs.images_dir, 'nanobench'),
  ])
  if api.vars.builder_cfg.get('cpu_or_gpu') == 'CPU' and 'Android' in b:
    assert api.flavor.device_dirs.texttraces_dir
    args.extend(['--texttraces', api.flavor.device_dirs.texttraces_dir])
  # Do not run svgs on Valgrind.
  if 'Valgrind' not in b:
    args.extend(['--svgs',  api.flavor.device_dirs.svg_dir])

  if upload_perf_results(b):
    now = api.time.utcnow()
    ts = int(calendar.timegm(now.utctimetuple()))
    json_path = api.flavor.device_path_join(
        api.flavor.device_dirs.perf_data_dir,
        'nanobench_%s_%d.json' % (api.properties['revision'], ts))
    args.extend(['--outResultsFile', json_path])

  api.run(api.flavor.step, 'nanobench', cmd=args,
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
  api.flavor.setup('nanobench')

  try:
    if all(v in api.vars.builder_name for v in ['Android', 'CPU']):
      api.flavor.install(skps=True, images=True, svgs=True, resources=True,
                         texttraces=True)
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
  'Perf-Android-Clang-NVIDIA_Shield-GPU-TegraX1-arm64-Release-All-Android',
  'Perf-Android-Clang-P30-GPU-MaliG76-arm64-Release-All-Android_Vulkan',
  'Perf-Android-Clang-Pixel3-GPU-Adreno630-arm64-Release-All-Android_Vulkan',
  'Perf-Android-Clang-Pixel3a-GPU-Adreno615-arm64-Release-All-Android',
  'Perf-ChromeOS-Clang-ASUSChromebookFlipC100-GPU-MaliT764-arm-Release-All',
  'Perf-ChromeOS-Clang-AcerChromebook13_CB5_311-GPU-TegraK1-arm-Release-All',
  'Perf-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All',
  'Perf-Debian9-Clang-GCE-CPU-AVX2-x86_64-Debug-All-ASAN',
  'Perf-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All-BonusConfigs',
  ('Perf-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All-'
   'SK_FORCE_RASTER_PIPELINE_BLITTER'),
  'Perf-Debian9-Clang-NUC5PPYH-GPU-IntelHD405-x86_64-Debug-All-Vulkan',
  'Perf-Debian9-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Release-All',
  ('Perf-Mac10.13-Clang-MacBook10.1-GPU-IntelHD615-x86_64-Release-All-'
   'CommandBuffer'),
  ('Perf-Mac10.13-Clang-MacBookPro11.5-GPU-RadeonHD8870M-x86_64-Release-All-'
   'Metal'),
  ('Perf-Mac10.13-Clang-MacMini7.1-GPU-IntelIris5100-x86_64-Release-All-'
   'CommandBuffer'),
  ('Perf-Ubuntu18-Clang-Golo-GPU-QuadroP400-x86_64-Release-All'
   '-Valgrind_SK_CPU_LIMIT_SSE41'),
  'Perf-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-ANGLE',
  'Perf-Win10-Clang-ShuttleA-GPU-GTX660-x86_64-Release-All-Vulkan',
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
