# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which analyzes a compiled binary for information (e.g. file size)

DEPS = [
  'checkout',
  'env',
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

  analyzed = 0
  with api.context(cwd=bin_dir):
    files = api.file.glob_paths(
        'find WASM binaries',
        bin_dir,
        '*.wasm',
        test_data=['pathkit.wasm'])
    analyzed += len(files)
    if files:
      analyze_wasm_file(api, checkout_root, out_dir, files)

    files = api.file.glob_paths(
        'find JS files',
        bin_dir,
        '*.js',
        test_data=['pathkit.js'])
    analyzed += len(files)
    if files:
      analyze_web_file(api, checkout_root, out_dir, files)

    files = api.file.glob_paths(
        'find JS mem files',
        bin_dir,
        '*.js.mem',
        test_data=['pathkit.js.mem'])
    analyzed += len(files)
    if files:
      analyze_web_file(api, checkout_root, out_dir, files)

    files = api.file.glob_paths(
        'find flutter library',
        bin_dir,
        'libflutter.so',
        test_data=['libflutter.so'])
    analyzed += len(files)
    if files:
      analyze_flutter_lib(api, checkout_root, out_dir, files)

    files = api.file.glob_paths(
        'find skia library',
        bin_dir,
        'libskia.so',
        test_data=['libskia.so'])
    analyzed += len(files)
    if files:
      analyze_cpp_lib(api, checkout_root, out_dir, files)

    files = api.file.glob_paths(
        'find skottie_tool',
        bin_dir,
        'skottie_tool',
        test_data=['skottie_tool'])
    analyzed += len(files)
    if files:
      make_treemap(api, checkout_root, out_dir, files)

  if not analyzed: # pragma: nocover
    raise Exception('No files were analyzed!')


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
      script = skia_dir.join('infra', 'bots', 'buildstats',
                             'buildstats_web.py')
      api.run(api.python, 'Analyze %s' % f, script=script,
          args=[f, out_dir, keystr, propstr])


# Get the raw size and a few metrics from bloaty
def analyze_cpp_lib(api, checkout_root, out_dir, files):
  (keystr, propstr) = keys_and_props(api)
  bloaty_exe = api.path['start_dir'].join('bloaty', 'bloaty')

  for f in files:
    skia_dir = checkout_root.join('skia')
    with api.context(cwd=skia_dir):
      script = skia_dir.join('infra', 'bots', 'buildstats',
                             'buildstats_cpp.py')
      api.run(api.python, 'Analyze %s' % f, script=script,
          args=[f, out_dir, keystr, propstr, bloaty_exe])


# Get the size of skia in flutter and a few metrics from bloaty
def analyze_flutter_lib(api, checkout_root, out_dir, files):
  (keystr, propstr) = keys_and_props(api)
  bloaty_exe = api.path['start_dir'].join('bloaty', 'bloaty')

  for f in files:

    skia_dir = checkout_root.join('skia')
    with api.context(cwd=skia_dir):
      stripped = api.vars.build_dir.join('libflutter_stripped.so')
      script = skia_dir.join('infra', 'bots', 'buildstats',
                             'buildstats_flutter.py')
      step_data = api.run(api.python, 'Analyze flutter', script=script,
                         args=[stripped, out_dir, keystr, propstr, bloaty_exe,
                               f],
                         stdout=api.raw_io.output())
      if step_data and step_data.stdout:
        magic_seperator = '#$%^&*'
        sections = step_data.stdout.split(magic_seperator)
        result = api.step.active_result
        logs = result.presentation.logs
        # Skip section 0 because it's everything before first print,
        # which is probably the empty string.
        logs['bloaty_file_symbol_short'] = sections[1].split('\n')
        logs['bloaty_file_symbol_full']  = sections[2].split('\n')
        logs['bloaty_symbol_file_short'] = sections[3].split('\n')
        logs['bloaty_symbol_file_full']  = sections[4].split('\n')
        logs['perf_json'] = sections[5].split('\n')


# Get the size of skia in flutter and a few metrics from bloaty
def analyze_wasm_file(api, checkout_root, out_dir, files):
  (keystr, propstr) = keys_and_props(api)
  bloaty_exe = api.path['start_dir'].join('bloaty', 'bloaty')

  for f in files:

    skia_dir = checkout_root.join('skia')
    with api.context(cwd=skia_dir):
      script = skia_dir.join('infra', 'bots', 'buildstats',
                             'buildstats_wasm.py')
      step_data = api.run(api.python, 'Analyze wasm', script=script,
                          args=[f, out_dir, keystr, propstr, bloaty_exe],
                          stdout=api.raw_io.output())
      if step_data and step_data.stdout:
        magic_seperator = '#$%^&*'
        sections = step_data.stdout.split(magic_seperator)
        result = api.step.active_result
        logs = result.presentation.logs
        # Skip section 0 because it's everything before first print,
        # which is probably the empty string.
        logs['bloaty_symbol_short'] = sections[1].split('\n')
        logs['bloaty_symbol_full']  = sections[2].split('\n')
        logs['perf_json']           = sections[3].split('\n')


# make a zip file containing an HTML treemap of the files
def make_treemap(api, checkout_root, out_dir, files):
  for f in files:
    env = {'DOCKER_CONFIG': '/home/chrome-bot/.docker'}
    with api.env(env):
      skia_dir = checkout_root.join('skia')
      with api.context(cwd=skia_dir):
        script = skia_dir.join('infra', 'bots', 'buildstats',
                               'make_treemap.py')
        api.run(api.python, 'Make code size treemap',
                             script=script,
                             args=[f, out_dir],
                             stdout=api.raw_io.output())


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
        stdout=api.raw_io.output('123456abc')) +
    api.step_data('Analyze wasm',
        stdout=api.raw_io.output(sample_wasm)) +
    api.step_data('Analyze flutter',
          stdout=api.raw_io.output(sample_flutter))
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
      ) +
    api.step_data('Analyze wasm',
        stdout=api.raw_io.output(sample_wasm)) +
    api.step_data('Analyze flutter',
          stdout=api.raw_io.output(sample_flutter))
  )

sample_wasm = """
#$%^&*
Report A
    Total size: 50 bytes
#$%^&*
Report B
    Total size: 60 bytes
#$%^&*
{
  "some": "json"
}
"""

sample_flutter = """
#$%^&*
Report A
    Total size: 50 bytes
#$%^&*
Report B
    Total size: 60 bytes
#$%^&*
Report C
    Total size: 70 bytes
#$%^&*
Report D
    Total size: 80 bytes
#$%^&*
{
  "some": "json"
}
"""
