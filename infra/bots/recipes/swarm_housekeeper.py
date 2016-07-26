# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for the Skia PerCommit Housekeeper.


DEPS = [
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'skia',
  'recipe_engine/step',
]


TEST_BUILDERS = {
  'client.skia.fyi': {
    'skiabot-linux-housekeeper-000': [
      'Housekeeper-PerCommit',
      'Housekeeper-PerCommit-Trybot',
    ],
  },
}


def RunSteps(api):
  # Checkout, compile, etc.
  api.skia.setup()

  cwd = api.path['checkout']

  api.skia.run(
    api.step,
    'android platform self-tests',
    cmd=['python',
         cwd.join('platform_tools', 'android', 'tests', 'run_all.py')],
    cwd=cwd,
    abort_on_failure=False)

  # TODO(borenet): Detect static initializers?

  gsutil_path = api.path['depot_tools'].join('third_party', 'gsutil',
                                             'gsutil')
  if not api.skia.is_trybot:
    api.skia.run(
      api.step,
      'generate and upload doxygen',
      cmd=['python', api.skia.resource('generate_and_upload_doxygen.py'),
           gsutil_path],
      cwd=cwd,
      abort_on_failure=False)

  cmd = ['python', api.skia.resource('run_binary_size_analysis.py'),
         '--library', api.skia.skia_out.join('Release', 'lib', 'libskia.so'),
         '--githash', api.properties['revision'],
         '--gsutil_path', gsutil_path]
  if api.skia.is_trybot:
    cmd.extend(['--issue_number', str(api.skia.m.properties['issue'])])
  api.skia.run(
    api.step,
    'generate and upload binary size data',
    cmd=cmd,
    cwd=cwd,
    abort_on_failure=False)

def GenTests(api):
  for mastername, slaves in TEST_BUILDERS.iteritems():
    for slavename, builders_by_slave in slaves.iteritems():
      for buildername in builders_by_slave:
        test = (
          api.test(buildername) +
          api.properties(buildername=buildername,
                         mastername=mastername,
                         slavename=slavename,
                         buildnumber=5,
                         revision='abc123',
                         path_config='kitchen',
                         swarm_out_dir='[SWARM_OUT_DIR]') +
          api.path.exists(api.path['slave_build'])
        )
        if 'Trybot' in buildername:
          test.properties['issue'] = '500'
        yield test
