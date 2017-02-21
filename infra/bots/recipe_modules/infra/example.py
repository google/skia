# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the Skia infra tests.


DEPS = [
  'core',
  'infra',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'run',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  api.core.checkout_steps()
  api.infra.update_go_deps()

  # Run the infra tests.
  infra_tests = api.vars.skia_dir.join(
      'infra', 'bots', 'infra_tests.py')
  with api.step.context({'cwd': api.vars.skia_dir}):
    api.step('infra_tests',
          cmd=['python', infra_tests],
          env=api.infra.go_env)


def GenTests(api):
  yield (
      api.test('infra_tests') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests',
                     mastername='client.skia.fyi',
                     slavename='dummy-slave',
                     buildnumber=5,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
    api.test('failed_one_update') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests',
                     mastername='client.skia.fyi',
                     slavename='dummy-slave',
                     buildnumber=5,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
    api.step_data('update go pkgs', retcode=1)
  )

  yield (
    api.test('failed_all_updates') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests',
                     mastername='client.skia.fyi',
                     slavename='dummy-slave',
                     buildnumber=5,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
    api.step_data('update go pkgs', retcode=1) +
    api.step_data('update go pkgs (attempt 2)', retcode=1) +
    api.step_data('update go pkgs (attempt 3)', retcode=1) +
    api.step_data('update go pkgs (attempt 4)', retcode=1) +
    api.step_data('update go pkgs (attempt 5)', retcode=1)
  )
