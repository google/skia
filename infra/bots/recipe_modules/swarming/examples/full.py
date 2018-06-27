# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# TODO(borenet): This module was copied from build.git and heavily modified to
# remove dependencies on other modules in build.git.  It belongs in a different
# repo. Remove this once it has been moved.


import json

DEPS = [
  'isolate',
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'swarming',
  'swarming_client',
]

from recipe_engine.recipe_api import Property

PROPERTIES = {
  'platforms': Property(default=('win',)),
  'show_isolated_out_in_collect_step': Property(default=True),
  'show_shards_in_collect_step': Property(default=False),
  'gtest_task': Property(default=False),
  #'isolated_script_task': Property(default=False),
  'merge': Property(default=None),
}

def RunSteps(api, platforms, show_isolated_out_in_collect_step,
             show_shards_in_collect_step, gtest_task, merge):
  # Checkout swarming client.
  api.swarming_client.checkout('master')

  # Ensure swarming_client version is fresh enough.
  api.swarming.check_client_version(step_test_data=(0, 8, 6))

  # Configure isolate & swarming modules (this is optional).
  api.isolate.isolate_server = 'https://isolateserver-dev.appspot.com'
  api.swarming.swarming_server = 'https://chromium-swarm-dev.appspot.com'
  api.swarming.add_default_tag('master:tryserver')
  api.swarming.default_expiration = 60*60
  api.swarming.default_hard_timeout = 60*60
  api.swarming.default_io_timeout = 20*60
  api.swarming.default_idempotent = True
  api.swarming.default_priority = 30
  api.swarming.default_user = 'joe'
  api.swarming.set_default_env('TESTING', '1')
  api.swarming.verbose = True

  api.swarming.set_default_dimension('inexistent', None)

  api.swarming.show_shards_in_collect_step = show_shards_in_collect_step
  api.swarming.show_isolated_out_in_collect_step = (
      show_isolated_out_in_collect_step)

  try:
    # Testing ReadOnlyDict.__setattr__() coverage.
    api.swarming.default_dimensions['invalid'] = 'foo'
  except TypeError:
    pass
  try:
    api.swarming.default_env['invalid'] = 'foo'
  except TypeError:
    pass

  # Create a temp dir to put *.isolated files into.
  temp_dir = api.path.mkdtemp('hello_isolated_world')

  # Prepare a bunch of swarming tasks to run hello_world on multiple platforms.
  tasks = []
  for platform in platforms:
    # Isolate example hello_world.isolate from swarming client repo.
    # TODO(vadimsh): Add a thin wrapper around isolate.py to 'isolate' module?
    step_result = api.python(
        'archive for %s' % platform,
        api.swarming_client.path.join('isolate.py'),
        [
          'archive',
          '--isolate', api.swarming_client.path.join(
              'example', 'payload', 'hello_world.isolate'),
          '--isolated', temp_dir.join('hello_world.isolated'),
          '--isolate-server', api.isolate.isolate_server,
          '--config-variable', 'OS', platform,
          '--verbose',
        ], stdout=api.raw_io.output_text())
    # TODO(vadimsh): Pass result from isolate.py though --output-json option.
    isolated_hash = step_result.stdout.split()[0].strip()

    # Create a task to run the isolated file on swarming, set OS dimension.
    # Also generate code coverage for multi-shard case by triggering multiple
    # shards on Linux.
    task = api.swarming.task('hello_world', isolated_hash,
                             task_output_dir=temp_dir.join('task_output_dir'))
    task.dimensions['os'] = api.swarming.prefered_os_dimension(platform)
    task.shards = 2 if platform == 'linux' else 1
    task.tags.add('os:' + platform)
    if api.swarming_client.get_script_version('swarming.py') >= (0, 8, 6):
      task.cipd_packages = [
          ('bin', 'super/awesome/pkg', 'git_revision:deadbeef')]
    tasks.append(task)

  # Launch all tasks.
  for task in tasks:
    step_result = api.swarming.trigger_task(task)
    assert step_result.swarming_task in tasks

  # Recipe can do something useful here locally while tasks are
  # running on swarming.
  api.step('local step', ['echo', 'running something locally'])

  # Wait for all tasks to complete.
  for task in tasks:
    step_result = api.swarming.collect_task(task)
    data = step_result.swarming.summary

    state = data['shards'][0]['state']
    if api.swarming.State.COMPLETED == state:
      state_name = api.swarming.State.to_string(state)
      assert 'Completed' == state_name, state_name
    assert step_result.swarming_task in tasks

  # Cleanup.
  api.file.rmtree('remove temp dir', temp_dir)


def GenTests(api):
  yield (
      api.test('basic') +
      api.step_data(
          'archive for win',
          stdout=api.raw_io.output_text('hash_for_win hello_world.isolated')) +
      api.step_data(
          'archive for linux',
          stdout=api.raw_io.output_text(
              'hash_for_linux hello_world.isolated')) +
      api.step_data(
          'archive for mac',
          stdout=api.raw_io.output_text('hash_for_mac hello_world.isolated')) +
      api.properties(platforms=('win', 'linux', 'mac')))

  yield (
      api.test('trybot') +
      api.step_data(
          'archive for win',
          stdout=api.raw_io.output_text('hash_for_win hello_world.isolated')) +
      api.properties(
          rietveld='https://codereview.chromium.org',
          issue='123',
          patchset='1001'))

  yield (
      api.test('show_shards_in_collect_step') +
      api.step_data(
          'archive for win',
          stdout=api.raw_io.output_text('hash_for_win hello_world.isolated')) +
      api.properties(
          rietveld='https://codereview.chromium.org',
          issue='123',
          patchset='1001',
          show_shards_in_collect_step=True))

  yield (
      api.test('show_isolated_out_in_collect_step') +
      api.step_data(
          'archive for win',
          stdout=api.raw_io.output_text('hash_for_win hello_world.isolated')) +
      api.properties(
          rietveld='https://codereview.chromium.org',
          issue='123',
          patchset='1001',
          show_isolated_out_in_collect_step=False))

  data = {
    'shards': [
      {
        '': '',
      }
    ]
  }

  data = {
    'shards': [
      {
        'abandoned_ts': '2014-09-25T01:41:00.123',
        'bot_id': 'vm30',
        'completed_ts': None,
        'created_ts': '2014-09-25T01:41:00.123',
        'durations': None,
        'exit_codes': [],
        'failure': False,
        'id': '148aa78d7aa0100',
        'internal_failure': False,
        'isolated_out': None,
        'modified_ts': '2014-09-25 01:42:00',
        'name': 'heartbeat-canary-2014-09-25_01:41:55-os=Windows',
        'outputs': [],
        'started_ts': '2014-09-25T01:42:11.123',
        'state': 0x30, # EXPIRED (old)
        'try_number': None,
        'user': 'unknown',
      }
    ],
  }

  yield (
      api.test('swarming_expired_old') +
      api.step_data(
          'archive for win',
          stdout=api.raw_io.output_text('hash_for_win hello_world.isolated')) +
      api.step_data('hello_world on Windows-7-SP1', api.swarming.summary(data)))

  data['shards'][0]['state'] = 'EXPIRED'
  yield (
      api.test('swarming_expired_new') +
      api.step_data(
          'archive for win',
          stdout=api.raw_io.output_text('hash_for_win hello_world.isolated')) +
      api.step_data('hello_world on Windows-7-SP1', api.swarming.summary(data)))

  data['shards'][0]['state'] = 0x40  # TIMED_OUT (old)
  yield (
      api.test('swarming_timeout_old') +
      api.step_data(
          'archive for win',
          stdout=api.raw_io.output_text('hash_for_win hello_world.isolated')) +
      api.step_data('hello_world on Windows-7-SP1', api.swarming.summary(data)))

  data['shards'][0]['state'] = 'TIMED_OUT'  # TIMED_OUT (old)
  yield (
      api.test('swarming_timeout_new') +
      api.step_data(
          'archive for win',
          stdout=api.raw_io.output_text('hash_for_win hello_world.isolated')) +
      api.step_data('hello_world on Windows-7-SP1', api.swarming.summary(data)))
