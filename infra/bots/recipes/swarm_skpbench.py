# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming skpbench.


DEPS = [
  'build/file',
  'core',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'recipe_engine/time',
  'run',
  'flavor',
  'vars',
]


TEST_BUILDERS = {
  'client.skia': {
    'skiabot-linux-swarm-000': [
      'Perf-Android-Clang-PixelC-GPU-TegraX1-arm64-Release-GN_Android_Skpbench',
      'Perf-Android-Clang-PixelC-CPU-TegraX1-arm64-Release-GN_Android_Skpbench',
    ],
  },
}


import calendar


def _run(api, title, *cmd, **kwargs):
  return api.run(api.step, title, cmd=list(cmd),
                 cwd=api.vars.skia_dir, **kwargs)


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

  skpbench_args = [
        api.path.join(api.vars.android_bin_dir, 'skpbench'),
        api.path.join(api.vars.android_data_dir, 'skps'),
        '--adb',
        '--resultsfile', table,
        '--config', 'gpu,esinst4']

  if 'GPU' in api.vars.builder_name:
    skpbench_args.append('--gpu')

  api.run(api.python, 'skpbench',
      script=skpbench_dir.join('skpbench.py'),
      args=skpbench_args)

  skiaperf_args = [
    table,
    '--properties',
    'gitHash',      api.vars.got_revision,
    'build_number', api.vars.build_number,
  ]

  skiaperf_args.extend(['no_buildbot', 'True'])
  skiaperf_args.extend(['swarming_bot_id', api.vars.swarming_bot_id])
  skiaperf_args.extend(['swarming_task_id', api.vars.swarming_task_id])

  now = api.time.utcnow()
  ts = int(calendar.timegm(now.utctimetuple()))
  api.file.makedirs('perf_dir', api.vars.perf_data_dir)
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


def GenTests(api):
  for mastername, slaves in TEST_BUILDERS.iteritems():
    for slavename, builders_by_slave in slaves.iteritems():
      for builder in builders_by_slave:
        test = (
          api.test(builder) +
          api.properties(buildername=builder,
                         mastername=mastername,
                         slavename=slavename,
                         buildnumber=5,
                         revision='abc123',
                         path_config='kitchen',
                         swarm_out_dir='[SWARM_OUT_DIR]') +
          api.path.exists(
              api.path['slave_build'].join('skia'),
              api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                           'skp', 'VERSION'),
          ) +
          api.step_data('get swarming bot id',
              stdout=api.raw_io.output('skia-bot-123')) +
          api.step_data('get swarming task id',
              stdout=api.raw_io.output('123456'))
        )

        yield test

