# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for Skia skpbench.


import calendar

PYTHON_VERSION_COMPATIBILITY = "PY3"

DEPS = [
  'flavor',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'recipe_engine/time',
  'run',
  'vars',
]

ADB_BINARY = 'adb.1.0.35'


def _run(api, title, *cmd, **kwargs):
  with api.context(cwd=api.path['start_dir'].join('skia')):
    return api.run(api.step, title, cmd=list(cmd), **kwargs)


def _adb(api, title, *cmd, **kwargs):
  if 'infra_step' not in kwargs:
    kwargs['infra_step'] = True
  return _run(api, title, ADB_BINARY, *cmd, **kwargs)


def skpbench_steps(api):
  """benchmark Skia using skpbench."""
  is_vulkan = 'Vulkan' in api.vars.builder_name
  is_metal = 'Metal' in api.vars.builder_name
  is_android = 'Android' in api.vars.builder_name
  is_apple_m1 = 'AppleM1' in api.vars.builder_name
  is_all_paths_volatile = 'AllPathsVolatile' in api.vars.builder_name
  is_mskp = 'Mskp' in api.vars.builder_name
  is_ddl = 'DDL' in api.vars.builder_name
  is_9x9 = '9x9' in api.vars.builder_name

  api.file.ensure_directory(
      'makedirs perf_dir', api.flavor.host_dirs.perf_data_dir)

  if is_android:
    app = api.vars.build_dir.join('skpbench')
    _adb(api, 'push skpbench', 'push', app, api.flavor.device_dirs.bin_dir)

  skpbench_dir = api.vars.workdir.join('skia', 'tools', 'skpbench')
  table = api.path.join(api.vars.swarming_out_dir, 'table')

  if is_vulkan:
    config = 'vk'
  elif is_metal:
    config = 'mtl'
  elif is_android:
    config = 'gles'
    if "MaliG77" in api.vars.builder_name:
      config = 'glesdmsaa,' + config
  else:
    config = 'gl'
    if "QuadroP400" in api.vars.builder_name or is_apple_m1:
      config = 'gldmsaa,' + config

  internal_samples = 4 if is_android or is_apple_m1 else 8

  if is_all_paths_volatile:
    config = "%smsaa%i" % (config, internal_samples)

  skpbench_invocation = api.path.join(api.flavor.device_dirs.bin_dir, 'skpbench')

  # skbug.com/10184
  if is_vulkan and 'GalaxyS20' in api.vars.builder_name:
    skpbench_invocation = "LD_LIBRARY_PATH=/data/local/tmp %s" % skpbench_invocation

  skpbench_args = [
        skpbench_invocation,
        '--resultsfile', table,
        '--config', config,
        '--internalSamples', str(internal_samples),
        # TODO(dogben): Track down what's causing bots to die.
        '-v5']
  if is_ddl:
    skpbench_args += ['--ddl']
    # disable the mask generation threads for simplicity's sake in DDL mode
    skpbench_args += ['--gpuThreads', '0']
  if is_9x9:
    skpbench_args += [
        '--ddlNumRecordingThreads', 9,
        '--ddlTilingWidthHeight', 3]
  if is_android:
    skpbench_args += [
        '--adb',
        '--adb_binary', ADB_BINARY]
  if is_mskp:
    skpbench_args += [api.flavor.device_dirs.mskp_dir]
  elif is_all_paths_volatile:
    skpbench_args += [
        '--allPathsVolatile',
        '--suffix', "_volatile",
        api.path.join(api.flavor.device_dirs.skp_dir, 'desk_*svg.skp'),
        api.path.join(api.flavor.device_dirs.skp_dir, 'desk_motionmark*.skp'),
        api.path.join(api.flavor.device_dirs.skp_dir, 'desk_chalkboard.skp')]
  else:
    skpbench_args += [api.flavor.device_dirs.skp_dir]

  if api.properties.get('dont_reduce_ops_task_splitting') == 'true':
    skpbench_args += ['--dontReduceOpsTaskSplitting']

  if api.properties.get('gpu_resource_cache_limit'):
    skpbench_args += ['--gpuResourceCacheLimit', api.properties.get('gpu_resource_cache_limit')]

  api.run(api.python, 'skpbench',
      script=skpbench_dir.join('skpbench.py'),
      args=skpbench_args)

  skiaperf_args = [
    table,
    '--properties',
    'gitHash', api.properties['revision'],
  ]
  if api.vars.is_trybot:
    skiaperf_args.extend([
      'issue',    api.vars.issue,
      'patchset', api.vars.patchset,
      'patch_storage', api.vars.patch_storage,
    ])

  skiaperf_args.extend(['swarming_bot_id', api.vars.swarming_bot_id])
  skiaperf_args.extend(['swarming_task_id', api.vars.swarming_task_id])

  now = api.time.utcnow()
  ts = int(calendar.timegm(now.utctimetuple()))
  json_path = api.path.join(
      api.flavor.host_dirs.perf_data_dir,
      'skpbench_%s_%d.json' % (api.properties['revision'], ts))

  skiaperf_args.extend([
    '--outfile', json_path
  ])

  skiaperf_args.append('--key')
  for k in sorted(api.vars.builder_cfg.keys()):
    if not k in ['configuration', 'role', 'is_trybot']:
      skiaperf_args.extend([k, api.vars.builder_cfg[k]])

  api.run(api.python, 'Parse skpbench output into Perf json',
      script=skpbench_dir.join('skiaperf.py'),
      args=skiaperf_args)


def RunSteps(api):
  api.vars.setup()
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)

  # The app_name passed to api.flavor.setup() is used to determine which app
  # to install on an attached device. That work is done in skpbench_steps, so
  # we pass None here.
  api.flavor.setup(None)

  try:
    mksp_mode = ('Mskp' in api.vars.builder_name)
    api.flavor.install(skps=not mksp_mode, mskps=mksp_mode)
    skpbench_steps(api)
  finally:
    api.flavor.cleanup_steps()
  api.run.check_failure()


TEST_BUILDERS = [
  'Perf-Android-Clang-Pixel2XL-GPU-Adreno540-arm64-Release-All-Android_Skpbench_Mskp',
  'Perf-Android-Clang-GalaxyS20-GPU-MaliG77-arm64-Release-All-Android_AllPathsVolatile_Skpbench',
  'Perf-Android-Clang-GalaxyS20-GPU-MaliG77-arm64-Release-All-Android_Vulkan_AllPathsVolatile_Skpbench',
  'Perf-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-Vulkan_Skpbench',
  'Perf-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-Vulkan_Skpbench_DDLTotal_9x9',
  'Perf-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-AllPathsVolatile_Skpbench',
  'Perf-Mac11-Clang-MacMini9.1-GPU-AppleM1-arm64-Release-All-Metal_AllPathsVolatile_Skpbench',
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
                                     'skp', 'VERSION'),
      ) +
      api.step_data('get swarming bot id',
          stdout=api.raw_io.output('skia-bot-123')) +
      api.step_data('get swarming task id',
          stdout=api.raw_io.output('123456'))
    )
    if 'Win' in builder:
      test += api.platform('win', 64)
    yield test

  b = ('Perf-Android-Clang-Pixel2XL-GPU-Adreno540-arm64-Release-All-'
       'Android_Vulkan_Skpbench')
  yield (
    api.test('trybot') +
    api.properties(buildername=b,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   dont_reduce_ops_task_splitting='true',
                   gpu_resource_cache_limit='16777216') +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                   'skp', 'VERSION'),
    ) +
    api.step_data('get swarming bot id',
        stdout=api.raw_io.output('skia-bot-123')) +
    api.step_data('get swarming task id',
        stdout=api.raw_io.output('123456')) +
    api.properties(patch_storage='gerrit') +
    api.properties.tryserver(
        buildername=b,
        gerrit_project='skia',
        gerrit_url='https://skia-review.googlesource.com/',
    )
  )
