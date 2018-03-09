# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for the Skia PerCommit Housekeeper.

DEPS = [
  'depot_tools/bot_update',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
  'core',
  'run',
  'vars',
]


def RunSteps(api):
  # Checkout, compile, etc.
  api.core.setup()

  cwd = api.path['checkout']

  # TODO(borenet): Detect static initializers?

  with api.context(cwd=cwd):
    gsutil_path = api.bot_update._module.PACKAGE_REPO_ROOT.join('gsutil.py')
    if not api.vars.is_trybot:
      api.run(
        api.step,
        'generate and upload doxygen',
        cmd=['python', api.core.resource('generate_and_upload_doxygen.py')],
        abort_on_failure=False)

    filename = 'binarysize_%s' % api.vars.got_revision
    if api.vars.is_trybot:
      filename += '_%s_%s' % (api.vars.issue, api.vars.patchset)
    dest_file = api.vars.perf_data_dir.join(filename)
    api.file.ensure_directory('makedirs perf_dir',
                              api.path.dirname(api.vars.perf_data_dir))
    cmd = ['python', api.core.resource('run_binary_size_analysis.py'),
           '--library', api.vars.skia_out.join('Release', 'libskia.so'),
           '--githash', api.vars.got_revision,
           '--dest', dest_file]
    if api.vars.is_trybot:
      cmd.extend(['--issue_number', str(api.properties['patch_issue'])])
    api.run(
      api.step,
      'generate and upload binary size data',
      cmd=cmd,
      abort_on_failure=False)


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
