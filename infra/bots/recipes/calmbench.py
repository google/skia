# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming calmbench.

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

def RunSteps(api):
  api.core.setup()
  api.flavor.install(skps=True, svgs=True)
  api.flavor.compile("most")
  with api.context(cwd=api.vars.skia_dir):
    extra_arg = '--svgs %s --skps %s' % (api.flavor.device_dirs.svg_dir,
                                         api.flavor.device_dirs.skp_dir)
    command = [
        'python', 'tools/calmbench/calmbench.py', 'modified',
        '--ninjadir', api.vars.skia_out.join("Release"),
        '--extraarg', extra_arg,
        '--writedir', api.vars.swarming_out_dir
    ]
    api.run(api.step, 'Run calmbench', cmd=command)
  api.run.check_failure()

def GenTests(api):
  builder = "Calmbench-Debian9"
  test = (
    api.test(builder) +
    api.properties(buildername=builder,
                   repository='https://skia.googlesource.com/skia.git',
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                   'svg', 'VERSION'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                   'skp', 'VERSION'),
    )
  )

  yield test
