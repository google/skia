# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the Skia presubmit.


DEPS = [
  'depot_tools/depot_tools',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'recipe_engine/uuid',
  'core',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  api.core.checkout_steps()

  with api.step.context({'cwd': api.vars.skia_dir}):
    # git-cl wants us to be on a branch.
    branch = 'tmp_%s' % api.uuid.random()
    api.step('create git branch',
             cmd=['git', 'checkout', '-b', branch])
    try:
      api.step('git status',
               cmd=['git', 'status'])
  
      depot_tools_path = api.depot_tools.package_repo_resource()
      env = {'PATH': api.path.pathsep.join([str(depot_tools_path), '%(PATH)s'])}
      api.step('presubmit',
               cmd=['git', 'cl', 'presubmit', '--force', '-v', '-v'],
               env=env)
    finally:
      api.step('git reset',
               cmd=['git', 'reset', '--hard', 'origin/master'])
      api.step('checkout origin/master',
               cmd=['git', 'checkout', 'origin/master'])
      api.step('delete git branch',
               cmd=['git', 'branch', '-D', branch])


def GenTests(api):
  yield (
      api.test('presubmit') +
      api.properties(buildername='Housekeeper-PerCommit-Presubmit',
                     mastername='client.skia.fyi',
                     slavename='dummy-slave',
                     buildnumber=5,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
