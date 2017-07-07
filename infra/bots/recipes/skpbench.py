# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for Skia skpbench.


import calendar


DEPS = [
  'core',
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


def _run(api, title, *cmd, **kwargs):
  with api.context(cwd=api.vars.skia_dir):
    return api.run(api.step, title, cmd=list(cmd), **kwargs)


def _adb(api, title, *cmd, **kwargs):
  if 'infra_step' not in kwargs:
    kwargs['infra_step'] = True
  return _run(api, title, 'adb', *cmd, **kwargs)


def skpbench_steps(api):
  """benchmark Skia using skpbench."""
  app = api.vars.skia_out.join(api.vars.configuration, 'skpbench')
  _adb(api, 'push skpbench', 'push', app, api.vars.android_bin_dir)

  skpbench_dir = api.vars.slave_dir.join('skia', 'tools', 'skpbench')
  table = api.path.join(api.vars.swarming_out_dir, 'table')

  config = 'gles,glesinst4'
  if 'Vulkan' in api.vars.builder_name:
    config = 'vk'

  skpbench_args = [
        api.path.join(api.vars.android_bin_dir, 'skpbench'),
        api.path.join(api.vars.android_data_dir, 'skps'),
        '--adb',
        '--resultsfile', table,
        '--config', config,
        # TODO(dogben): Track down what's causing bots to die.
        '-v', '5']

  api.run(api.python, 'skpbench',
      script=skpbench_dir.join('skpbench.py'),
      args=skpbench_args)

  skiaperf_args = [
    table,
    '--properties',
    'gitHash',      api.vars.got_revision,
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
  api.file.ensure_directory('makedirs perf_dir', api.vars.perf_data_dir)
  json_path = api.path.join(
      api.vars.perf_data_dir,
      'skpbench_%s_%d.json' % (api.vars.got_revision, ts))

  skiaperf_args.extend([
    '--outfile', json_path
  ])

  keys_blacklist = ['configuration', 'role', 'is_trybot']
  skiaperf_args.append('--key')
  for k in sorted(api.vars.builder_cfg.keys()):
    if not k in keys_blacklist:
      skiaperf_args.extend([k, api.vars.builder_cfg[k]])

  api.run(api.python, 'Parse skpbench output into Perf json',
      script=skpbench_dir.join('skiaperf.py'),
      args=skiaperf_args)


def RunSteps(api):
  api.core.setup()
  try:
    api.flavor.install(skps=True)
    skpbench_steps(api)
  finally:
    api.flavor.cleanup_steps()
  api.run.check_failure()


TEST_BUILDERS = [
  'Perf-Android-Clang-PixelC-GPU-TegraX1-arm64-Release-Android_Skpbench',
  ('Perf-Android-Clang-PixelC-GPU-TegraX1-arm64-Release-'
   'Android_Vulkan_Skpbench'),
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

    yield test

  b = 'Perf-Android-Clang-PixelC-GPU-TegraX1-arm64-Release-Android_Skpbench'
  yield (
    api.test('trybot') +
    api.properties(buildername=b,
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
        stdout=api.raw_io.output('123456')) +
    api.properties(patch_storage='gerrit') +
    api.properties.tryserver(
        buildername=b,
        gerrit_project='skia',
        gerrit_url='https://skia-review.googlesource.com/',
    )
  )
