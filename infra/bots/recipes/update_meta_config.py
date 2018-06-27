# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Recipe for the Bot that updates meta config."""


DEPS = [
  'depot_tools/gclient',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'core',
  'infra',
  'run',
  'vars',
]


TEST_BUILDERS = {
  'client.skia.compile': {
    'skiabot-linux-swarm-000': [
      'Housekeeper-Nightly-UpdateMetaConfig',
    ],
  },
}


UPDATE_META_CONFIG_GITCOOKIES_FILE = 'update_meta_config.git_cookies'
UPDATE_META_CONFIG_KEY = 'update_meta_config_git_cookies'


def RunSteps(api):
  api.core.setup()

  if api.vars.is_trybot:
    raise Exception('Cannot run update_meta_config recipe as a trybot')
  update_meta_config_gitcookies = api.path.join(
      api.path.expanduser('~'), UPDATE_META_CONFIG_GITCOOKIES_FILE)
  repo_name = api.properties.get('repository').split('/')[-1].rstrip('.git')
  cmd = ['python',
         api.vars.skia_dir.join('infra', 'bots', 'update_meta_config.py'),
         '--repo_name', repo_name,
         '--tasks_json', api.vars.skia_dir.join('infra', 'bots', 'tasks.json'),
         '--gitcookies', str(update_meta_config_gitcookies)]
  with api.infra.MetadataFetch(
      api, UPDATE_META_CONFIG_KEY, UPDATE_META_CONFIG_GITCOOKIES_FILE):
    with api.context(cwd=api.vars.skia_dir):
      api.run(api.step, 'Update meta/config', cmd=cmd)


def GenTests(api):
  builder = 'Housekeeper-Nightly-UpdateMetaConfig'
  yield (
      api.test(builder) +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('failed_update') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('Update meta/config', retcode=1)
  )

  yield (
      api.test('trybot_test') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     patch_issue=123,
                     patch_set=3) +
      api.expect_exception('Exception')
  )
