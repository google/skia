# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the Skia infra tests.


DEPS = [
  'checkout',
  'infra',
  'recipe_engine/context',
  'recipe_engine/properties',
  'recipe_engine/step',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  checkout_root = api.checkout.default_checkout_root
  api.checkout.bot_update(checkout_root=checkout_root)

  # Run the infra tests.
  repo_name = api.properties['repository'].split('/')[-1]
  if repo_name.endswith('.git'):
    repo_name = repo_name[:-len('.git')]
  repo_root = checkout_root.join(repo_name)
  infra_tests = repo_root.join('infra', 'bots', 'infra_tests.py')
  python = 'python'
  if 'Win' in api.properties['buildername']:
    python = 'python.exe'
    api.step('where git', cmd=['where', 'git'])
  with api.context(cwd=checkout_root.join(repo_name),
                   env=api.infra.go_env):
    api.step('infra_tests', cmd=[python, infra_tests])


def GenTests(api):
  yield (
      api.test('infra_tests') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests_Win',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
