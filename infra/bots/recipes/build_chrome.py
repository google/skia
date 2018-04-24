# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Build Chrome with latest Skia."""


DEPS = [
  'core',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/properties',
  'recipe_engine/step',
  'run',
  'vars',
]


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

    # Copy the binary into the output dir.
    api.file.copy(
        'copy out/Release/chrome',
        out_dir.join('chrome'),
        api.vars.build_dir.join('chrome'))


def GenTests(api):
  builder = 'Housekeeper-PerCommit-BuildChrome'
  yield (
      api.test(builder) +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
