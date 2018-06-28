# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe for Skia Swarming compute testing.

DEPS = [
  'flavor',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'run',
  'vars',
]

def RunSteps(api):
  api.vars.setup()
  api.flavor.setup()

  api.run(api.flavor.step, 'hello-opencl', cmd=['hello-opencl'])

  api.run.check_failure()

def GenTests(api):
  builder = ('Test-Debian9-Clang-NUC7i5BNK-GPU-IntelIris640-x86_64-Debug-All'
             '-OpenCL')
  yield (
      api.test(builder) +
      api.properties(buildername=builder,
                     buildbucket_build_id='123454321',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
