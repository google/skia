# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Recipe for the Skia-Fuchsia Canary."""


DEPS = [
  'build/file',
  'core',
  'depot_tools/gclient',
  'infra',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'vars',
]


TEST_BUILDERS = {
  'client.skia.compile': {
    'skiabot-linux-swarm-000': [
      'Canary-Fuchsia-Ubuntu-Clang-x86_64-Release',
    ],
  },
}


def RunSteps(api):
  # Check out Chrome.
  api.core.setup()

  src_dir = api.vars.checkout_root

  # Call jiri
  platform = 'linux64'  # This bot only runs on linux; don't bother checking.

  api.step('download jiri bootstrap script',
           ['curl', '-o', src_dir.join('bootstrap_jiri'),
           'https://raw.githubusercontent.com/fuchsia-mirror/jiri/master/scripts/bootstrap_jiri'],
           cwd=src_dir)

  api.step('run jiri bootstrap script',
           [src_dir.join('bootstrap_jiri'), 'fuchsia'],
           cwd=src_dir)

  jiri = src_dir.join('.jiri_root', 'bin', 'jiri')
  fuchsia_dir = src_dir.join('fuchsia')

  api.step('jiri import',
           [jiri, 'import', 'fuchsia', 'https://fuchsia.googlesource.com/manifest'],
           cwd=fuchsia_dir)

  api.step('jiri update',
           [jiri, 'update'],
           cwd=fuchsia_dir)

  # print.
  api.step('Print folder contents',
           ['ls', '-ahl'],
           cwd=fuchsia_dir)



def GenTests(api):
  mastername = 'client.skia.compile'
  slavename = 'skiabot-linux-swarm-000'
  builder = 'Canary-Fuchsia-Ubuntu-Clang-x86_64-Release'
  yield (
      api.test(builder) +
      api.properties(buildername=builder,
                     mastername=mastername,
                     slavename=slavename,
                     revision='abc123',
                     buildnumber=2,
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
