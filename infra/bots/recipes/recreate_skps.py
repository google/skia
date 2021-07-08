# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Recipe for the Skia RecreateSKPs Bot."""


DEPS = [
  'checkout',
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
      'Housekeeper-Nightly-RecreateSKPs_DryRun',
      'Housekeeper-Weekly-RecreateSKPs',
    ],
  },
}


def RunSteps(api):
  # Check out Chrome.
  api.vars.setup()

  checkout_root = api.checkout.default_checkout_root
  extra_gclient_env = {
      'CPPFLAGS': '-DSK_ALLOW_CROSSPROCESS_PICTUREIMAGEFILTERS=1'}
  api.checkout.bot_update(
      checkout_root=checkout_root,
      checkout_chromium=True,
      extra_gclient_env=extra_gclient_env)

  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)

  src_dir = checkout_root.join('src')
  skia_dir = checkout_root.join('skia')
  out_dir = src_dir.join('out', 'Release')

  with api.context(cwd=skia_dir):
    # Fetch `sk`
    api.run(api.step, 'Fetch SK',
            cmd=['python3', skia_dir.join('bin', 'fetch-sk')])

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

  with api.context(cwd=skia_dir):
    # Recreate SKPs.
    script = skia_dir.join('infra', 'bots', 'assets', 'skp',
                           'create_and_upload.py')
    cmd = [
      'vpython3', '-u', script,
      '--chrome_src_path', src_dir,
      '--browser_executable', out_dir.join('chrome'),
    ]
    if 'DryRun' in api.properties['buildername']:
      cmd.append('--dry_run')
    api.run(api.step, 'Recreate SKPs', cmd=cmd)

    if 'DryRun' in api.properties['buildername']:
      return

    # Regenerate tasks.json.
    api.run(api.step, 'Regenerate tasks.json',
            cmd=['go', 'run', './infra/bots/gen_tasks.go'])
    
    # Upload a CL.
    commit_msg = '''Update SKP version

Automatic commit by the RecreateSKPs bot.
'''
    api.run(api.step, 'git commit',
            cmd=['git', 'commit', '-a', '-m', commit_msg])
    cmd = [
      'git', 'cl', 'upload',
      '--bypass-hooks', '--use-commit-queue',
      '--tbrs=rmistry@google.com',
    ]
    api.run(api.step, 'git cl upload', cmd=cmd)


def GenTests(api):
  builder = 'Housekeeper-Nightly-RecreateSKPs_DryRun'
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
      api.path.exists(api.path['start_dir'].join('skp_output'))
  )

  yield (
      api.test('failed_recreate') +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'].join('skp_output')) +
      api.step_data('Recreate SKPs', retcode=1)
  )
