# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming compile.


DEPS = [
  'git',
  'recipe_engine/properties',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  api.git('config', '--global', '--list')
  api.git('status')


def GenTests(api):
  yield (
    api.test('git_test') +
    api.properties(buildername='Build-Win-MSVC-x86_64-Release-Vulkan',
                   swarm_out_dir='[SWARM_OUT_DIR]')
  )
