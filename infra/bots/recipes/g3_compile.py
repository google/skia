# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/raw_io',
  'recipe_engine/step',
]


def RunSteps(api):
  buildername = api.properties['buildername']
  issue = api.properties.get('patch_issue')
  patchset = api.properties.get('patch_set')
  if not issue or not patchset:
    raise Exception('%s can only be run as a trybot.' % buildername)

  infrabots_dir = api.path['start_dir'].join('skia', 'infra', 'bots')
  trigger_wait_g3_script = infrabots_dir.join('g3_compile',
                                              'trigger_wait_g3_task.py')

  # Trigger a compile task and wait for it to complete.
  cmd = ['python', trigger_wait_g3_script,
         '--issue', issue,
         '--patchset', patchset,
        ]
  try:
    api.step('Trigger and wait for g3 compile task', cmd=cmd)
  except api.step.StepFailure as e:
    # Add withpatch and nopatch logs as links (if they exist).
    gs_file = 'gs://g3-compile-tasks/%s-%s.json' % (issue, patchset)
    step_result = api.step(
        'Get cl link', ['gsutil', 'cat', gs_file], stdout=api.json.output())
    task_json = step_result.stdout
    if task_json.get('cl'):
      api.step.active_result.presentation.links['CL link'] = task_json['cl']
    raise e


def GenTests(api):
  yield(
    api.test('g3_compile_trybot') +
    api.properties(
        buildername='Build-Debian9-Clang-TAP-Presubmit-G3_Framework',
        path_config='kitchen',
        swarm_out_dir='[SWARM_OUT_DIR]',
        repository='https://skia.googlesource.com/skia.git',
        patch_issue=1234,
        patch_set=1,
        revision='abc123',
    )
  )

  yield(
    api.test('g3_compile_trybot_failure') +
    api.properties(
        buildername='Build-Debian9-Clang-TAP-Presubmit-G3_Framework',
        path_config='kitchen',
        swarm_out_dir='[SWARM_OUT_DIR]',
        repository='https://skia.googlesource.com/skia.git',
        patch_issue=1234,
        patch_set=1,
        revision='abc123',
    ) +
    api.step_data('Trigger and wait for g3 compile task', retcode=1) +
    api.step_data('Get cl link', stdout=api.raw_io.output('{"cl":"cl/12345"}'))
  )

  yield(
    api.test('g3_compile_nontrybot') +
    api.properties(
        buildername='Build-Debian9-Clang-TAP-Presubmit-G3_Framework',
        path_config='kitchen',
        swarm_out_dir='[SWARM_OUT_DIR]',
        repository='https://skia.googlesource.com/skia.git',
        revision='abc123',
    ) +
    api.expect_exception('Exception')
  )
