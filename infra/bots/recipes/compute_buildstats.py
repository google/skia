# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which analyzes a compiled binary for information (e.g. file size)

DEPS = [
  'checkout',
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

def RunSteps(api):
  api.vars.setup()

  checkout_root = api.checkout.default_checkout_root
  api.checkout.bot_update(checkout_root=checkout_root)

  out_dir = api.vars.swarming_out_dir
  # Any binaries to scan should be here.
  bin_dir = api.vars.build_dir

  api.file.ensure_directory('mkdirs out_dir', out_dir, mode=0777)

  with api.context(cwd=bin_dir):
    files = api.file.glob_paths(
        'find WASM binaries',
        bin_dir,
        '*.wasm',
        test_data=['pathkit.wasm'])
    if len(files):
      analyze_web_file(api, checkout_root, out_dir, files)

    files = api.file.glob_paths(
        'find JS files',
        bin_dir,
        '*.js',
        test_data=['pathkit.js'])
    if len(files):
      analyze_web_file(api, checkout_root, out_dir, files)

    files = api.file.glob_paths(
        'find JS mem files',
        bin_dir,
        '*.js.mem',
        test_data=['pathkit.js.mem'])
    if len(files):
      analyze_web_file(api, checkout_root, out_dir, files)


def keys_and_props(api):
  keys = []
  keys_blacklist = ['role']
  for k in sorted(api.vars.builder_cfg.keys()):
      if not k in keys_blacklist:
        keys.extend([k, api.vars.builder_cfg[k]])
  keystr = ' '.join(keys)

  props = [
    'gitHash', api.properties['revision'],
    'swarming_bot_id', api.vars.swarming_bot_id,
    'swarming_task_id', api.vars.swarming_task_id,
  ]

  if api.vars.is_trybot:
    props.extend([
      'issue',    str(api.vars.issue),
      'patchset', str(api.vars.patchset),
      'patch_storage', api.vars.patch_storage,
    ])
  propstr = ' '.join(props)
  return (keystr, propstr)


# Get the raw and gzipped size of the given file
def analyze_web_file(api, checkout_root, out_dir, files):
  (keystr, propstr) = keys_and_props(api)

  for f in files:
    skia_dir = checkout_root.join('skia')
    with api.context(cwd=skia_dir):
      script = skia_dir.join('infra', 'bots', 'buildstats_web.py')
      api.run(api.python, 'Analyze %s' % f, script=script,
        args=[f, out_dir, keystr, propstr])


def GenTests(api):
  builder = 'BuildStats-Debian9-EMCC-wasm-Release-PathKit'
  yield (
    api.test('normal_bot') +
    api.properties(buildername=builder,
                   repository='https://skia.googlesource.com/skia.git',
                   revision='abc123',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   path_config='kitchen') +
      api.step_data('get swarming bot id',
          stdout=api.raw_io.output('skia-bot-123')) +
      api.step_data('get swarming task id',
          stdout=api.raw_io.output('123456abc'))
  )

  yield (
    api.test('trybot') +
    api.properties(buildername=builder,
                   repository='https://skia.googlesource.com/skia.git',
                   revision='abc123',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   patch_repo='https://skia.googlesource.com/skia.git',
                   path_config='kitchen') +
    api.step_data('get swarming bot id',
        stdout=api.raw_io.output('skia-bot-123')) +
    api.step_data('get swarming task id',
        stdout=api.raw_io.output('123456abc')) +
    api.properties(patch_storage='gerrit') +
    api.properties.tryserver(
        buildername=builder,
        gerrit_project='skia',
        gerrit_url='https://skia-review.googlesource.com/',
      )
  )
