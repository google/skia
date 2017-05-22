# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# TODO(borenet): This module was copied from build.git and heavily modified to
# remove dependencies on other modules in build.git.  It belongs in a different
# repo. Remove this once it has been moved.


from recipe_engine import recipe_test_api

import state

class SwarmingTestApi(recipe_test_api.RecipeTestApi):

  @recipe_test_api.placeholder_step_data
  def summary(self, data):
    return self.m.json.output(data)

  def canned_summary_output(
      self, shards=1, failure=False, internal_failure=False):
    return self.summary({
      'shards': [
        {
          'abandoned_ts': None,
          'bot_id': 'vm30',
          'completed_ts': '2014-09-25T01:42:00.123',
          'created_ts': '2014-09-25T01:41:00.123',
          'durations': [5.7, 31.5],
          'exit_codes': [0, 0],
          'failure': failure,
          'id': '148aa78d7aa%02d00' % i,
          'internal_failure': internal_failure,
          'isolated_out': {
            'isolated': 'abc123',
            'isolatedserver': 'https://isolateserver.appspot.com',
            'namespace': 'default-gzip',
            'view_url': 'blah',
          },
          'modified_ts': '2014-09-25 01:42:00',
          'name': 'heartbeat-canary-2014-09-25_01:41:55-os=Windows',
          'outputs': [
            'Heart beat succeeded on win32.\n',
            'Foo',
          ],
          'outputs_ref': {
            'view_url': 'blah',
          },
          'started_ts': '2014-09-25T01:42:11.123',
          'state': state.State.COMPLETED,
          'try_number': 1,
          'user': 'unknown',
        } for i in xrange(shards)
      ],
    })
