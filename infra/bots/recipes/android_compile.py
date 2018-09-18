# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import math


DEPS = [
  'recipe_engine/path',
  'recipe_engine/properties',
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

  # Trigger a compile task on android-compile.skia.org and wait for it to
  # complete.
  cmd = ['python', trigger_wait_ac_script,
         '--issue', issue,
         '--patchset', patchset,
        ]
  api.step('Trigger and wait for task on android-compile.skia.org', cmd=cmd)


def GenTests(api):
  yield(
    api.test('android_compile_trybot') +
    api.properties(
        buildername='Build-Debian9-Clang-gce_x86_phone-eng-Android_Framework',
        path_config='kitchen',
        swarm_out_dir='[SWARM_OUT_DIR]',
        repository='https://skia.googlesource.com/skia.git',
        patch_issue=1234,
        patch_set=1,
    )
  )

  yield(
    api.test('android_compile_nontrybot') +
    api.properties(
        buildername='Build-Debian9-Clang-gce_x86_phone-eng-Android_Framework',
        path_config='kitchen',
        swarm_out_dir='[SWARM_OUT_DIR]',
        repository='https://skia.googlesource.com/skia.git',
        revision='abc123',
    ) +
    api.expect_exception('Exception')
  )
