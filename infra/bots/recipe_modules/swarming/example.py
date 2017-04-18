# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'swarming',
]


def RunSteps(api):
  api.swarming.setup('mydir', swarming_rev='abc123')
  api.swarming.create_isolated_gen_json(
      'isolate_path', 'isolate_dir', 'linux', 'task', {'myvar': 'myval'})
  tasks_to_hashes = api.swarming.batcharchive(targets=[
      'task-%s' % num for num in range(5)])
  tasks = api.swarming.trigger_swarming_tasks(
      tasks_to_hashes, dimensions={'os': 'Linux'})
  for t in tasks:
    api.swarming.collect_swarming_task(t)


def GenTests(api):
  yield (
      api.test('test') +
      api.properties(revision='abc123')
  )
