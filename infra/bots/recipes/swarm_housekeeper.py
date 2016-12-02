# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for the Skia PerCommit Housekeeper.

DEPS = [
  'core',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
  'run',
  'vars',
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
  api.core.setup()

  cwd = api.path['checkout']

  api.run(
    api.step,
    'android platform self-tests',
    cmd=['python',
         cwd.join('platform_tools', 'android', 'tests', 'run_all.py')],
    cwd=cwd,
    abort_on_failure=False)

  # TODO(borenet): Detect static initializers?

  gsutil_path = api.path['depot_tools'].join('gsutil.py')
  if not api.vars.is_trybot:
    api.run(
      api.step,
      'generate and upload doxygen',
      cmd=['python', api.core.resource('generate_and_upload_doxygen.py')],
      cwd=cwd,
      abort_on_failure=False)

  cmd = ['python', api.core.resource('run_binary_size_analysis.py'),
         '--library', api.vars.skia_out.join(
             'Release', 'lib', 'libskia.so'),
         '--githash', api.properties['revision'],
         '--gsutil_path', gsutil_path]
  if api.vars.is_trybot:
    cmd.extend(['--issue_number', str(api.properties['patch_issue'])])
  api.run(
    api.step,
    'generate and upload binary size data',
    cmd=cmd,
    cwd=cwd,
    abort_on_failure=False)

  env = {}
  env['GOPATH'] = api.vars.tmp_dir.join('golib')
  extractexe = env['GOPATH'].join('bin', 'extract_comments')
  goexe = api.vars.slave_dir.join('go', 'go', 'bin', 'go')

  # Compile extract_comments.
  api.run(
    api.step,
    'compile extract_comments',
    cmd=[goexe, 'get', 'go.skia.org/infra/comments/go/extract_comments'],
    cwd=cwd,
    env=env,
    abort_on_failure=True)

  # Run extract_comments on the gm directory.
  api.run(
    api.step,
    'run extract_comments',
    cmd=[extractexe, '--dir', 'gm', '--dest', 'gs://skia-doc/gm/comments.json'],
    cwd=cwd,
    env=env,
    abort_on_failure=True)


def GenTests(api):
  yield (
      api.test('Housekeeper-PerCommit') +
      api.properties(buildername='Housekeeper-PerCommit',
                     mastername='client.skia.fyi',
                     slavename='skiabot-linux-housekeeper-000',
                     buildnumber=5,
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'])
  )
  yield (
      api.test('Housekeeper-PerCommit-Trybot') +
      api.properties(buildername='Housekeeper-PerCommit',
                     mastername='client.skia.fyi',
                     slavename='skiabot-linux-housekeeper-000',
                     buildnumber=5,
                     revision='abc123',
                     path_config='kitchen',
                     patch_storage='gerrit',
                     nobuildbot='True',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.properties.tryserver(
          buildername='Housekeeper-PerCommit',
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      ) +
      api.path.exists(api.path['start_dir'])
  )
