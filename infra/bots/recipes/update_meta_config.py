# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Recipe for the Bot that updates meta config."""


DEPS = [
  'build/file',
  'depot_tools/gclient',
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
      'Housekeeper-Nightly-Update_Meta_Config',
    ],
  },
}


UPDATE_META_CONFIG_GITCOOKIES_FILE = 'update_meta_config.git_cookies'
UPDATE_META_CONFIG_KEY = 'update_meta_config_git_cookies'


class gitcookies_auth(object):
  """Download update-meta-config@skia.org's .gitcookies."""
  def __init__(self, api, metadata_key):
    self.m = api
    self._key = metadata_key

  def __enter__(self):
    return self.m.python.inline(
        'download update_meta_config.gitcookies',
        """
import os
import urllib2

TOKEN_FILE = '%s'
TOKEN_URL = 'http://metadata/computeMetadata/v1/project/attributes/%s'

req = urllib2.Request(TOKEN_URL, headers={'Metadata-Flavor': 'Google'})
contents = urllib2.urlopen(req).read()

home = os.path.expanduser('~')
token_file = os.path.join(home, TOKEN_FILE)

with open(token_file, 'w') as f:
  f.write(contents)
        """ % (UPDATE_META_CONFIG_GITCOOKIES_FILE,
               self._key),
    )

  def __exit__(self, t, v, tb):
    self.m.python.inline(
        'cleanup update_meta_config.gitcookies',
        """
import os


TOKEN_FILE = '%s'


home = os.path.expanduser('~')
token_file = os.path.join(home, TOKEN_FILE)
if os.path.isfile(token_file):
  os.remove(token_file)
        """ % (UPDATE_META_CONFIG_GITCOOKIES_FILE),
    )
    return v is None


def RunSteps(api):
  api.core.setup()

  update_meta_config_gitcookies = api.path.join(
      api.path.expanduser('~'), UPDATE_META_CONFIG_GITCOOKIES_FILE)
  cmd = ['python',
         api.vars.skia_dir.join('infra', 'bots', 'update_meta_config.py'),
         '--repo_name', 'skia',
         '--tasks_json', api.vars.skia_dir.join('infra', 'bots', 'tasks.json'),
         '--gitcookies', str(update_meta_config_gitcookies)]
  with gitcookies_auth(api, UPDATE_META_CONFIG_KEY):
    with api.step.context({'cwd': api.vars.skia_dir}):
      api.run(api.step, 'Update meta/config', cmd=cmd)


def GenTests(api):
  builder = 'Housekeeper-Nightly-Update_Meta_Config'
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
