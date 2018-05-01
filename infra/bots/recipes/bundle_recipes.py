# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming compile.


DEPS = [
  'git',
  'recipe_engine/context',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
]


def RunSteps(api):
  bundle_dir = api.properties['swarm_out_dir'] + '/recipe_bundle'
  skia_dir = api.path['start_dir'].join('skia')
  recipes_py = api.path['start_dir'].join('skia', 'infra', 'bots', 'recipes.py')
  with api.git.env():
    with api.context(cwd=skia_dir):
      api.step('git init', infra_step=True,
               cmd=['git', 'init'])
      api.step('git add', infra_step=True,
               cmd=['git', 'add', '.'])
      api.step('git commit', infra_step=True,
               cmd=['git', 'commit', '-m', 'commit recipes'])
      api.step('Bundle Recipes', infra_step=True,
               cmd=['python', recipes_py, 'bundle',
                    '--destination', bundle_dir])


def GenTests(api):
  yield (
    api.test('BundleRecipes') +
    api.properties(buildername='Housekeeper-PerCommit-BundleRecipes',
                   swarm_out_dir='[SWARM_OUT_DIR]')
  )
