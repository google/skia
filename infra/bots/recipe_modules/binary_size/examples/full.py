# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'binary_size',
  'recipe_engine/path',
  'recipe_engine/properties',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  dest_file = api.path['start_dir'].join('binary_size')
  api.binary_size.run_analysis(api.path['start_dir'], dest_file)


def GenTests(api):
  yield (
      api.test('binary_size') +
      api.properties(buildername='Housekeeper-PerCommit',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.properties.tryserver(
          buildername='Housekeeper-PerCommit',
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )
  )
