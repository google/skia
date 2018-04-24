# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Recipe for the Skia RecreateSKPs Bot."""


DEPS = [
  'core',
  'infra',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'run',
  'vars',
]


def RunSteps(api):
  # Check out Chrome.
  api.core.setup()

  src_dir = api.vars.checkout_root.join('src')
  chrome = api.vars.build_dir.join('chrome')

  # Clean up the output dir.
  output_dir = api.path['start_dir'].join('skp_output')
  api.file.ensure_directory('makedirs skp_output', output_dir)

  # Capture the SKPs.
  asset_dir = api.vars.infrabots_dir.join('assets', 'skp')
  cmd = ['--chrome_src_path', src_dir,
         '--browser_executable', chrome,
         '--target_dir', output_dir]
  # TODO(rmistry): Uncomment the below after skbug.com/6797 is fixed.
  # if 'Canary' not in api.properties['buildername']:
  #   cmd.append('--upload_to_partner_bucket')
  with api.context(cwd=api.vars.skia_dir):
    api.run(api.python, 'Recreate SKPs', script=asset_dir.join('create.py'),
            args=cmd)

  # Upload the SKPs.
  if 'Canary' not in api.properties['buildername']:
    api.infra.update_go_deps()
    cmd = ['--target_dir', output_dir]
    with api.context(cwd=api.vars.skia_dir, env=api.infra.go_env):
      api.run(api.python, 'Upload SKPs',
              script=api.vars.infrabots_dir.join('upload_skps.py'), args=cmd)


def GenTests(api):
  builder = 'Housekeeper-Nightly-RecreateSKPs_Canary'
  yield (
      api.test(builder) +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  builder = 'Housekeeper-Weekly-RecreateSKPs'
  yield (
      api.test(builder) +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
