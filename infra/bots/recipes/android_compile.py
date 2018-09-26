# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import math


DEPS = [
  'recipe_engine/context',
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
    # This bot currently only supports trybot runs because:
    # Non-trybot runs could fail if the Android tree is red. We mitigate this
    # for trybot runs by verifying that runs without the patch succeed. We do
    # not currently have a way to do the same for non-trybot runs.
    raise Exception('%s can only be run as a trybot.' % buildername)

  infrabots_dir = api.path['start_dir'].join('skia', 'infra', 'bots')
  trigger_wait_ac_script = infrabots_dir.join('android_compile',
                                              'trigger_wait_ac_task.py')

  # Trigger a compile task on the android compile server and wait for it to
  # complete.
  cmd = ['python', trigger_wait_ac_script,
         '--issue', issue,
         '--patchset', patchset,
        ]
  try:
    api.step('Trigger and wait for task on android compile server', cmd=cmd)
  except api.step.StepFailure as e:
    # Add withpatch and nopatch logs as links (if they exist).
    gs_file = 'gs://android-compile-tasks/%s-%s.json' % (issue, patchset)
    step_result = api.step('Get task log links',
                           ['gsutil', 'cat', gs_file],
                           stdout=api.json.output())
    task_json = step_result.stdout
    if task_json.get('withpatch_log'):
      api.step.active_result.presentation.links[
          'withpatch compilation log link'] = task_json['withpatch_log']
    if task_json.get('nopatch_log'):
      api.step.active_result.presentation.links[
          'nopatch compilation log link'] = task_json['nopatch_log']
    raise e


def GenTests(api):
  yield(
    api.test('android_compile_trybot') +
    api.properties(
        buildername='Build-Debian9-Clang-cf_x86_phone-eng-Android_Framework',
        path_config='kitchen',
        swarm_out_dir='[SWARM_OUT_DIR]',
        repository='https://skia.googlesource.com/skia.git',
        patch_issue=1234,
        patch_set=1,
    )
  )

  yield(
    api.test('android_compile_trybot_failure') +
    api.properties(
        buildername='Build-Debian9-Clang-cf_x86_phone-eng-Android_Framework',
        path_config='kitchen',
        swarm_out_dir='[SWARM_OUT_DIR]',
        repository='https://skia.googlesource.com/skia.git',
        patch_issue=1234,
        patch_set=1,
    ) +
    api.step_data('Trigger and wait for task on android compile server',
                  retcode=1) +
    api.step_data('Get task log links',
                  stdout=api.raw_io.output(
                      '{"withpatch_log":"link1", "nopatch_log":"link2"}'))
  )

  yield(
    api.test('android_compile_nontrybot') +
    api.properties(
        buildername='Build-Debian9-Clang-cf_x86_phone-eng-Android_Framework',
        path_config='kitchen',
        swarm_out_dir='[SWARM_OUT_DIR]',
        repository='https://skia.googlesource.com/skia.git',
        revision='abc123',
    ) +
    api.expect_exception('Exception')
  )
