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
  swarming_out_dir = api.vars.make_path(api.properties['swarm_out_dir'])
  bundle_dir = swarming_out_dir.join('recipe_bundle')
  recipes_py = api.path['start_dir'].join('skia', 'infra', 'bots', 'recipes.py')
  api.step('Bundle Recipes', infra_step=True,
           cmd=['python', recipes_py, 'bundle', '--destination', bundle_dir])

  api.run.check_failure()


def GenTests(api):
  yield (
    api.test('BundleRecipes') +
    api.properties(buildername='Housekeeper-PerCommit-BundleRecipes',
                   swarm_out_dir='[SWARM_OUT_DIR]')
  )
