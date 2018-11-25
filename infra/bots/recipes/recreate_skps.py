# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Recipe for the Skia RecreateSKPs Bot."""


DEPS = [
  'core',
  'depot_tools/gclient',
  'infra',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'run',
  'vars',
]


TEST_BUILDERS = {
  'client.skia.compile': {
    'skiabot-linux-swarm-000': [
      'Housekeeper-Nightly-RecreateSKPs_Canary',
      'Housekeeper-Weekly-RecreateSKPs',
    ],
  },
}


UPDATE_SKPS_GITCOOKIES_FILE = 'update_skps.git_cookies'

UPDATE_SKPS_GITCOOKIES_GS_PATH = (
    'gs://skia-buildbots/artifacts/server/.gitcookies_update-skps')


def RunSteps(api):
  # Check out Chrome.
  api.core.setup()

  src_dir = api.vars.checkout_root.join('src')
  out_dir = src_dir.join('out', 'Release')

  with api.context(cwd=src_dir):
    # Call GN.
    platform = 'linux64'  # This bot only runs on linux; don't bother checking.
    gn = src_dir.join('buildtools', platform, 'gn')
    gn_env = {'CPPFLAGS': '-DSK_ALLOW_CROSSPROCESS_PICTUREIMAGEFILTERS=1',
              'GYP_GENERATORS': 'ninja'}
    with api.context(env=gn_env):
      api.run(api.step, 'GN', cmd=[gn, 'gen', out_dir])

    # Build Chrome.
    api.run(api.step, 'Build Chrome', cmd=['ninja', '-C', out_dir, 'chrome'])

  # Clean up the output dir.
  output_dir = api.path['start_dir'].join('skp_output')
  if api.path.exists(output_dir):
    api.run.rmtree(output_dir)
  api.file.ensure_directory('makedirs skp_output', output_dir)

  # Capture the SKPs.
  asset_dir = api.vars.infrabots_dir.join('assets', 'skp')
  cmd = ['python', asset_dir.join('create.py'),
         '--chrome_src_path', src_dir,
         '--browser_executable', src_dir.join('out', 'Release', 'chrome'),
         '--target_dir', output_dir]
  # TODO(rmistry): Uncomment the below after skbug.com/6797 is fixed.
  # if 'Canary' not in api.properties['buildername']:
  #   cmd.append('--upload_to_partner_bucket')
  with api.context(cwd=api.vars.skia_dir):
    api.run(api.step, 'Recreate SKPs', cmd=cmd)

  # Upload the SKPs.
  if 'Canary' not in api.properties['buildername']:
    api.infra.update_go_deps()
    update_skps_gitcookies = api.path['start_dir'].join(
        UPDATE_SKPS_GITCOOKIES_FILE)
    cmd = ['python',
           api.vars.skia_dir.join('infra', 'bots', 'upload_skps.py'),
           '--target_dir', output_dir,
           '--gitcookies', str(update_skps_gitcookies)]
    with api.infra.DownloadGitCookies(
        UPDATE_SKPS_GITCOOKIES_GS_PATH, update_skps_gitcookies, api):
      with api.context(cwd=api.vars.skia_dir, env=api.infra.go_env):
        api.run(api.step, 'Upload SKPs', cmd=cmd)


def GenTests(api):
  builder = 'Housekeeper-Nightly-RecreateSKPs_Canary'
  yield (
      api.test(builder) +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'].join('skp_output'))
  )

  builder = 'Housekeeper-Weekly-RecreateSKPs'
  yield (
      api.test(builder) +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'].join('skp_output')) +
      api.path.exists(api.path['start_dir'].join(UPDATE_SKPS_GITCOOKIES_FILE))
  )

  yield (
      api.test('failed_upload') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'].join('skp_output')) +
      api.step_data('Upload SKPs', retcode=1)
  )
