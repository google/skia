# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  if not api.vars.is_trybot:
    raise Exception('%s can only be run as a trybot.' % api.vars.builder_name)

  infrabots_dir = api.path['start_dir'].join('skia', 'infra', 'bots')
  trigger_wait_g3_script = infrabots_dir.join('g3_compile',
                                              'trigger_wait_g3_task.py')

  output_dir = api.path.mkdtemp('g3_try')
  output_file = output_dir.join('output_file')
  # Trigger a compile task and wait for it to complete.
  cmd = ['python', trigger_wait_g3_script,
         '--issue', api.vars.issue,
         '--patchset', api.vars.patchset,
         '--output_file', output_file,
        ]
  try:
    api.step('Trigger and wait for g3 compile task', cmd=cmd)
  except api.step.StepFailure as e:
    # Add CL link if it exists in the output_file.
    task_json = api.file.read_json(
        'Read task json', output_file, test_data={'cl': 12345})
    if task_json.get('cl'):
      api.step.active_result.presentation.links['CL link'] = (
          'http://cl/%d' % task_json['cl'])
    raise e


def GenTests(api):
  yield(
    api.test('g3_compile_trybot') +
    api.properties.tryserver(
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
    ) +
    api.properties(
        buildername='Build-Debian9-Clang-TAP-Presubmit-G3_Framework',
        path_config='kitchen',
        swarm_out_dir='[SWARM_OUT_DIR]',
        repository='https://skia.googlesource.com/skia.git',
        revision='abc123',
    )
  )

  yield(
    api.test('g3_compile_trybot_failure') +
    api.properties.tryserver(
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
    ) +
    api.properties(
        buildername='Build-Debian9-Clang-TAP-Presubmit-G3_Framework',
        path_config='kitchen',
        swarm_out_dir='[SWARM_OUT_DIR]',
        repository='https://skia.googlesource.com/skia.git',
        revision='abc123',
    ) +
    api.step_data('Trigger and wait for g3 compile task', retcode=1)
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
