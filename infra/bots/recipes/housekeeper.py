# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for the Skia PerCommit Housekeeper.


import calendar


DEPS = [
  'core',
  'depot_tools/bot_update',
  'flavor',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
  'recipe_engine/time',
  'run',
  'vars',
]


def RunSteps(api):
  # Checkout, compile, etc.
  api.vars.setup()
  api.core.checkout_bot_update()
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)
  api.flavor.setup()

  cwd = api.path['checkout']

  # TODO(borenet): Detect static initializers?

  with api.context(cwd=cwd):
    if not api.vars.is_trybot:
      api.run(
        api.step,
        'generate and upload doxygen',
        cmd=['python', api.core.resource('generate_and_upload_doxygen.py')],
        abort_on_failure=False)

    now = api.time.utcnow()
    ts = int(calendar.timegm(now.utctimetuple()))
    filename = 'nanobench_%s_%d.json' % (api.vars.got_revision, ts)
    dest_dir = api.vars.perf_data_dir
    dest_file = dest_dir + '/' + filename
    api.file.ensure_directory('makedirs perf_dir', dest_dir)
    cmd = ['python', api.core.resource('run_binary_size_analysis.py'),
           '--library', api.vars.skia_out.join('Release', 'libskia.so'),
           '--githash', api.properties['revision'],
           '--dest', dest_file]
    if api.vars.is_trybot:
      cmd.extend(['--issue_number', str(api.properties['patch_issue'])])
    api.run(
      api.step,
      'generate binary size data',
      cmd=cmd)


def GenTests(api):
  yield (
      api.test('Housekeeper-PerCommit') +
      api.properties(buildername='Housekeeper-PerCommit',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'])
  )
  yield (
      api.test('Housekeeper-PerCommit-Trybot') +
      api.properties(buildername='Housekeeper-PerCommit',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     patch_storage='gerrit',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.properties.tryserver(
          buildername='Housekeeper-PerCommit',
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      ) +
      api.path.exists(api.path['start_dir'])
  )
