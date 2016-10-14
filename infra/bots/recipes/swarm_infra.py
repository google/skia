# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the Skia infra tests.


DEPS = [
  'core',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  api.core.checkout_steps()

  gopath = api.vars.checkout_root.join('gopath')
  env = {'GOPATH': gopath}
  api.step('update_go_pkgs',
           cmd=['go', 'get', '-u', 'go.skia.org/infra/...'],
           env=env)

  infra_tests = api.vars.skia_dir.join(
      'infra', 'bots', 'infra_tests.py')
  api.step('infra_tests',
           cmd=['python', infra_tests],
           cwd=api.vars.skia_dir,
           env=env)


def GenTests(api):
  yield (
      api.test('infra_tests') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests',
                     mastername='client.skia.fyi',
                     slavename='dummy-slave',
                     buildnumber=5,
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
