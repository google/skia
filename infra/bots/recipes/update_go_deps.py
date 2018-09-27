# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Recipe for updating the go_deps asset."""


DEPS = [
  'checkout',
  'infra',
  'recipe_engine/context',
  'recipe_engine/properties',
  'recipe_engine/python',
  'run',
  'vars',
]


def RunSteps(api):
  api.vars.setup()

  checkout_root = api.checkout.default_checkout_root
  api.checkout.bot_update(checkout_root=checkout_root)

  skia_dir = checkout_root.join('skia')
  with api.context(cwd=skia_dir, env=api.infra.go_env):
    script = skia_dir.join('infra', 'bots', 'update_go_deps.py')
    api.run(api.python, 'Update Asset', script=script)


def GenTests(api):
  builder = 'Housekeeper-Nightly-UpdateGoDEPS'
  yield (
      api.test(builder) +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
