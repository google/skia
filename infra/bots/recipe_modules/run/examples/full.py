# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

PYTHON_VERSION_COMPATIBILITY = "PY3"

DEPS = [
  'recipe_engine/context',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/step',
  'run',
  'vars',
]


def myfunc(api, i):
  api.run(api.step, 'run %d' % i, cmd=['echo', str(i)])


def RunSteps(api):
  api.vars.setup()
  try:
    api.run(api.step, 'fail', cmd=['false'])
  except api.step.StepFailure:
    pass
  api.run(api.step, 'fail again', cmd=['false'], abort_on_failure=False)
  api.run(api.step, 'do a thing', cmd=['echo', 'do the thing'])
  assert len(api.run.failed_steps) == 2

  # Run once.
  for i in range(10):
    api.run.run_once(myfunc, api, i)

  # Read and write files.
  api.run.readfile('myfile.txt')
  api.run.writefile('myfile.txt', 'contents')
  api.run.rmtree('mydir')
  api.run.asset_version('my_asset', api.vars.cache_dir.join('work', 'skia'))

  # Merge PATHs.
  with api.context(env={'PATH': 'mydir:%(PATH)s'}):
    api.run(api.step, 'env', cmd=['env'])

  def between_attempts_fn(attempt):
    api.run(api.step, 'between_attempts #%d' % attempt,
            cmd=['echo', 'between_attempt'])

  # Retries.
  try:
    api.run.with_retry(api.step, 'retry fail', 5, cmd=['false'],
                       between_attempts_fn=between_attempts_fn)
  except api.step.StepFailure:
    pass
  assert len(api.run.failed_steps) == 7

  api.run.with_retry(api.step, 'retry success', 3, cmd=['false'],
                     between_attempts_fn=between_attempts_fn)
  assert len(api.run.failed_steps) == 7

  # Check failure.
  api.run.check_failure()


def GenTests(api):
  buildername = 'Build-Win-Clang-x86_64-Release-Vulkan'
  yield (
      api.test('test') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.platform('win', 64) +
      api.step_data('fail', retcode=1) +
      api.step_data('fail again', retcode=1) +
      api.step_data('retry fail', retcode=1) +
      api.step_data('retry fail (attempt 2)', retcode=1) +
      api.step_data('retry fail (attempt 3)', retcode=1) +
      api.step_data('retry fail (attempt 4)', retcode=1) +
      api.step_data('retry fail (attempt 5)', retcode=1) +
      api.step_data('retry success', retcode=1) +
      api.step_data('retry success (attempt 2)', retcode=1)
    )
