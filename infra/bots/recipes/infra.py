# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the Skia infra tests.

PYTHON_VERSION_COMPATIBILITY = "PY3"

DEPS = [
  'infra',
  'recipe_engine/context',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'vars',
]


def git_init(api, repo_root, env):
  with api.context(cwd=repo_root, env=env):
    # Some tests assume that they're being run inside a git repo.
    api.step('git init', cmd=['git', 'init'])
    api.step('git add .', cmd=['git', 'add', '.'])
    api.step('git commit', cmd=['git', 'commit', '-a', '-m', 'initial commit'])


def RunSteps(api):
  api.vars.setup()

  # Run the infra tests.
  repo_name = api.properties['repository'].split('/')[-1]
  if repo_name.endswith('.git'):
    repo_name = repo_name[:-len('.git')]
  repo_root = api.path['start_dir'].join(repo_name)
  infra_tests = repo_root.join('infra', 'bots', 'infra_tests.py')

  # Merge the default environment with the Go environment.
  env = {}
  env.update(api.infra.go_env)
  for k, v in api.vars.default_env.items():
    # The PATH variable gets merged; all others get replaced.
    if k == 'PATH':
      # This works because the value for PATH in go_env and default_env includes
      # the '%(PATH)s' placeholder.
      env[k] = env[k] % {k: v}
    else:
      env[k] = v

  git_init(api, repo_root, env)
  if repo_name != 'skia':
    git_init(api, api.path['start_dir'].join('skia'), env)

  with api.context(cwd=repo_root, env=env):
    # Unfortunately, the recipe tests are flaky due to file removal on Windows.
    # Run multiple attempts.
    last_exc = None
    for _ in range(3):
      try:
        api.step('infra_tests', cmd=['python', '-u', infra_tests])
        break
      except api.step.StepFailure as e:  # pragma: nocover
        last_exc = e
    else:  # pragma: nocover
      raise last_exc

def GenTests(api):
  yield (
      api.test('infra_tests') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests_Linux',
                     repository='https://skia.googlesource.com/skia.git',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
  yield (
      api.test('infra_tests_lottie_ci') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests_Linux',
                     repository='https://skia.googlesource.com/lottie-ci.git',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
