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
  # api.flavor.install(skps=True)
  with api.context(cwd=api.vars.skia_dir):
    api.step('Run calmbench',
             ['python', 'tools/calmbench/calmbench.py', 'master'])
  api.run.check_failure()

def GenTests(api):
  builder = "Calmbench-Debian9"
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
    )# +
    # api.step_data('get swarming bot id',
    #     stdout=api.raw_io.output('skia-bot-123')) +
    # api.step_data('get swarming task id',
    #     stdout=api.raw_io.output('123456'))
  )

  yield test
