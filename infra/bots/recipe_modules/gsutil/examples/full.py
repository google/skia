# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the Skia gsutils tests.

PYTHON_VERSION_COMPATIBILITY = "PY2+3"

DEPS = [
  'gsutil',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
  'run',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  api.gsutil.cp('test file', '/foo/file', 'gs://bar-bucket/file',
                extra_args=['-Z'], multithread=True)

def GenTests(api):
  yield (
      api.test('gsutil_tests') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('gsutil_win_tests') +
      api.properties(buildername='Test-Win10-MSVC-ShuttleA-GPU-GTX660-x86_64-Debug-All',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
    api.test('failed_one_upload') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
    api.step_data('upload test file', retcode=1)
  )

  yield (
    api.test('failed_all_uploads') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
    api.step_data('upload test file', retcode=1) +
    api.step_data('upload test file (attempt 2)', retcode=1) +
    api.step_data('upload test file (attempt 3)', retcode=1) +
    api.step_data('upload test file (attempt 4)', retcode=1) +
    api.step_data('upload test file (attempt 5)', retcode=1)
  )
