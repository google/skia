# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming compile.


DEPS = [
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'skia-recipes/compile',
]


def RunSteps(api):
  api.compile.run()


def GenTests(api):
  yield (
    api.test('Build-Mac-Clang-Arm7-Release') +
    api.properties(buildername='Build-Mac-Clang-Arm7-Release-iOS',
                   mastername='fake-master',
                   slavename='fake-slave',
                   buildnumber=5,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
    api.path.exists(
        api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
    )
  )
