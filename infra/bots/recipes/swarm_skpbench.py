# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming skpbench.


DEPS = [
  'core',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'run',
  'flavor',
  'vars',
]


TEST_BUILDERS = {
  'client.skia': {
    'skiabot-linux-swarm-000': [
      'Perf-Android-Clang-PixelC-GPU-TegraX1-arm64-Release-GN_Android_Skpbench',
    ],
  },
}

# Data should go under in _data_dir, which may be preserved across runs.
_data_dir = '/sdcard/revenge_of_the_skiabot/'
# Executables go under _bin_dir, which, well, allows executable files.
_bin_dir  = '/data/local/tmp/'


def _strip_environment(api):
  api.vars.default_env = {k: v for (k,v)
                             in api.vars.default_env.iteritems()
                             if k in ['PATH']}


def _run(api, title, *cmd, **kwargs):
  _strip_environment(api)
  return api.run(api.step, title, cmd=list(cmd),
                 cwd=api.vars.skia_dir, **kwargs)


def _adb(api, title, *cmd, **kwargs):
  if 'infra_step' not in kwargs:
    kwargs['infra_step'] = True
  return _run(api, title, 'adb', *cmd, **kwargs)


def skpbench_steps(api):
  """benchmark Skia using skpbench."""
  app = api.vars.skia_out.join(api.vars.configuration, 'skpbench')
  _adb(api, 'push skpbench', 'push', app, _bin_dir)

  intermediate_table = api.path.join(api.vars.swarming_out_dir, 'table.txt')

  api.run(api.python, 'Run Skpbench on device',
      script=api.vars.local_skpbench_dir.join('skpbench.py'),
      args=[
        '--adb',
        '--resultsfile', intermediate_table,
        api.path.join(_bin_dir, 'skpbench'),
        api.path.join(_data_dir, 'skps')])

  args = [
    intermediate_table,
    '--properties',
    'gitHash',      api.vars.got_revision,
    'build_number', api.vars.build_number,
  ]

  args.extend(['no_buildbot', 'True'])
  args.extend(['swarming_bot_id', api.vars.swarming_bot_id])
  args.extend(['swarming_task_id', api.vars.swarming_task_id])

  args.extend([
    '--outfile', api.path.join(api.vars.swarming_out_dir, 'skpbench.json')
  ])

  keys_blacklist = ['configuration', 'role', 'is_trybot']
  args.append('--key')
  for k in sorted(api.vars.builder_cfg.keys()):
    if not k in keys_blacklist:
      args.extend([k, api.vars.builder_cfg[k]])

  api.run(api.python, 'Run Skpbench on device',
      script=api.vars.local_skpbench_dir.join('skiaperf.py'),
      args=args)


def RunSteps(api):
  api.core.setup()
  try:
    api.flavor.install(images=False, svgs=False)
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

