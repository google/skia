# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the Skia presubmit.


DEPS = [
  'core',
  'recipe_engine/properties',
  'recipe_engine/step',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  api.core.checkout_steps()
  api.step('presubmit',
           cmd=['git', 'cl', 'presubmit', '--force'],
           cwd=api.vars.skia_dir)


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
