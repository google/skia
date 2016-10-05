# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the Skia presubmit.


DEPS = [
  'core',
  'depot_tools/depot_tools',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'recipe_engine/uuid',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  api.core.checkout_steps()

  api.step('git status',
           cmd=['git', 'status'],
           cwd=api.vars.skia_dir)

  depot_tools_path = api.depot_tools.package_repo_resource()
  script = depot_tools_path.join('presubmit_support.py')
  env = {'PATH': api.path.pathsep.join([str(depot_tools_path), '%(PATH)s'])}
  # TODO(borenet): --upstream=HEAD^ is a hack to force presubmit_support to
  # find a diff. Otherwise, it quits early with:
  #  "Warning, no PRESUBMIT.py found."
  api.step('presubmit',
           cmd=[script, '--commit', '--upstream=HEAD^', '-v', '-v'],
           cwd=api.vars.skia_dir,
           env=env)


def GenTests(api):
  yield (
      api.test('presubmit') +
      api.properties(buildername='Housekeeper-PerCommit-Presubmit',
                     mastername='client.skia.fyi',
                     slavename='dummy-slave',
                     buildnumber=5,
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
