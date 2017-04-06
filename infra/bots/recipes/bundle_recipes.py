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
  recipes_py = api.vars.infrabots_dir.join('recipes.py')
  api.run(api.step, 'Bundle Recipes', infra_step=True,
          cmd=['python', recipes_py, 'bundle'])
  src_dir = api.path['start_dir'].join('bundle')
  dst_dir = api.vars.swarming_out_dir.join('recipe_bundle')
  api.shutil.copytree('Copy Recipe Bundle', src_dir, dst_dir)
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
                   swarm_out_dir='[SWARM_OUT_DIR]') +
    api.path.exists(
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    )
  )
