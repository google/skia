# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming skpbench.


import calendar

from recipe_engine import recipe_api


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

  config = 'gpu,esinst4'
  if 'Vulkan' in api.vars.builder_name:
    config = 'vk'

  skpbench_args = [
        api.path.join(api.vars.android_bin_dir, 'skpbench'),
        api.path.join(api.vars.android_data_dir, 'skps'),
        '--adb',
        '--resultsfile', table,
        '--config', config]

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


class SkpBenchApi(recipe_api.RecipeApi):
  def run(self):
    self.m.core.setup()
    try:
      self.m.flavor.install(skps=True)
      skpbench_steps(self.m)
    finally:
      self.m.flavor.cleanup_steps()
    self.m.run.check_failure()
