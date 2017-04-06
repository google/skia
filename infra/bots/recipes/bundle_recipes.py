# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming compile.


DEPS = [
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/shutil',
  'recipe_engine/step',
  'core',
  'run',
  'vars',
]


def RunSteps(api):
  api.core.setup()

  bundle_dir = api.vars.swarming_out_dir.join('recipe_bundle')
  with api.step.context({'cwd': api.vars.skia_dir}):
    if api.vars.is_trybot:
      # Recipe bundling requires that any changes be committed.
      api.run(api.step, 'Commit Patch', infra_step=True,
              cmd=['git', 'commit', '-a', '-m', 'Commit Patch'])
    recipes_py = api.vars.infrabots_dir.join('recipes.py')
    api.run(api.step, 'Bundle Recipes', infra_step=True,
            cmd=['python', recipes_py, 'bundle', '--destination', bundle_dir])

  api.run.check_failure()


def GenTests(api):
  yield (
    api.test('BundleRecipes') +
    api.properties(buildername='Housekeeper-PerCommit-BundleRecipes',
                   mastername='fake-master',
                   slavename='fake-slave',
                   buildnumber=5,
                   repository='https://skia.googlesource.com/skia.git',
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   nobuildbot='True',
                   patch_issue='10101',
                   patch_set='3') +
    api.path.exists(
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    )
  )
