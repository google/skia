# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming skpbench.


DEPS = [
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/raw_io',
  'skpbench',
]


def RunSteps(api):
  api.skpbench.run()


def GenTests(api):
  b = 'Perf-Android-Clang-PixelC-GPU-TegraX1-arm64-Release-GN_Android_Skpbench'
  yield (
    api.test(b) +
    api.properties(buildername=b,
                   mastername='fake-master',
                   slavename='fake-slave',
                   buildnumber=5,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]') +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                   'skp', 'VERSION'),
    ) +
    api.step_data('get swarming bot id',
        stdout=api.raw_io.output('skia-bot-123')) +
    api.step_data('get swarming task id',
        stdout=api.raw_io.output('123456'))
  )
