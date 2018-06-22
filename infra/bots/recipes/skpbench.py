# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for Skia skpbench.


import calendar


DEPS = [
  'flavor',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
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
  api.file.ensure_directory(
      'makedirs perf_dir', api.flavor.host_dirs.perf_data_dir)

  if 'Android' in api.vars.builder_name:
    app = api.vars.build_dir.join('skpbench')
    _adb(api, 'push skpbench', 'push', app, api.flavor.device_dirs.bin_dir)

  skpbench_dir = api.vars.slave_dir.join('skia', 'tools', 'skpbench')
  table = api.path.join(api.vars.swarming_out_dir, 'table')

  if 'Vulkan' in api.vars.builder_name:
    config = 'vk'
  else:
    config = 'gles'

  skpbench_args = [
        api.path.join(api.flavor.device_dirs.bin_dir, 'skpbench'),
        '--resultsfile', table,
        '--config', config,
        # TODO(dogben): Track down what's causing bots to die.
        '-v5']
  if 'DDL' in api.vars.builder_name:
    # This adds the "--ddl" flag for both DDLTotal and DDLRecord
    skpbench_args += ['--ddl']
  if 'DDLRecord' in api.vars.builder_name:
    skpbench_args += ['--ddlRecord']
  if '9x9' in api.vars.builder_name:
    skpbench_args += [
        '--ddlNumAdditionalThreads', 9,
        '--ddlTilingWidthHeight', 3]
  if 'Android' in api.vars.builder_name:
    skpbench_args += [
        '--adb',
        '--adb_binary', ADB_BINARY]
  if 'CCPR' in api.vars.builder_name:
    skpbench_args += [
        '--pr', 'ccpr',
        '--nocache',
        api.path.join(api.flavor.device_dirs.skp_dir, 'desk_*svg.skp'),
        api.path.join(api.flavor.device_dirs.skp_dir, 'desk_chalkboard.skp')]
  else:
    skpbench_args += [api.flavor.device_dirs.skp_dir]

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

  keys = ['os', 'compiler', 'model', 'cpu_or_gpu', 'cpu_or_gpu_value', 'arch',
          'test_filter', 'extra_tokens']
  skiaperf_args.append('--key')
  for k in sorted(keys):
    key = k
    value = api.properties[k]
    if key == 'extra_tokens':
      key = 'extra_config'
      value = '_'.join(value.split(','))
    skiaperf_args.append(key)
    skiaperf_args.append(value)

  api.run(api.python, 'Parse skpbench output into Perf json',
      script=skpbench_dir.join('skiaperf.py'),
      args=skiaperf_args)


def RunSteps(api):
  api.vars.setup()
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)

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

  try:
    api.flavor.install(skps=True)
    skpbench_steps(api)
  finally:
    api.flavor.cleanup_steps()
  api.run.check_failure()


TEST_BUILDERS = [
  ('Perf-Android-Clang-Pixel-GPU-Adreno530-arm64-Release-All-'
   'Android_CCPR_Skpbench'),
  'Perf-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-Vulkan_Skpbench',
  ('Perf-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-'
   'Vulkan_Skpbench_DDLTotal_9x9'),
  ('Perf-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-'
   'Vulkan_Skpbench_DDLRecord_9x9'),
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
      if tok == 'SK':  # pragma: no cover
        extra_tokens_list.append('_'.join(extra_split[idx:]))
        break
      else:
        extra_tokens_list.append(tok)
  extra_tokens = ','.join(extra_tokens_list)

  return dict(
    arch=arch,
    buildername=buildername,
    buildbucket_build_id='123454321',
    compiler=compiler,
    configuration=configuration,
    cpu_or_gpu=cpu_or_gpu,
    cpu_or_gpu_value=cpu_or_gpu_value,
    extra_tokens=extra_tokens,
    model=model,
    os=os,
    revision='abc123',
    path_config='kitchen',
    swarm_out_dir='[SWARM_OUT_DIR]',
    test_filter=test_filter,
  )


def GenTests(api):
  for builder in TEST_BUILDERS:
    test = (
      api.test(builder) +
      api.properties(**defaultProps(builder)) +
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

    yield test

  b = ('Perf-Android-Clang-Pixel2XL-GPU-Adreno540-arm64-Release-All-'
       'Android_Vulkan_Skpbench')
  yield (
    api.test('trybot') +
    api.properties(**defaultProps(b)) +
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
