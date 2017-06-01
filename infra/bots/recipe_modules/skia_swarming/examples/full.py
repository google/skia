# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


DEPS = [
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'skia_swarming',
]


def RunSteps(api):
  api.skia_swarming.setup('mydir', swarming_rev='abc123')
  api.skia_swarming.create_isolated_gen_json(
      'isolate_path', 'isolate_dir', 'linux', 'task', {'myvar': 'myval'},
      blacklist=['*.pyc'])
  tasks_to_hashes = api.skia_swarming.batcharchive(targets=[
      'task-%s' % num for num in range(5)])
  tasks = api.skia_swarming.trigger_swarming_tasks(
      tasks_to_hashes, dimensions={'os': 'Linux'}, extra_args=['--extra'])
  for t in tasks:
    api.skia_swarming.collect_swarming_task(t)


def GenTests(api):
  yield (
      api.test('test') +
      api.properties(revision='abc123')
  )
