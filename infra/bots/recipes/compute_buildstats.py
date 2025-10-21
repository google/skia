# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which analyzes a compiled binary for information (e.g. file size)

import ast
import json


DEPS = [
  'checkout',
  'env',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'run',
  'vars',
]


MAGIC_SEPERATOR = '#$%^&*'
TOTAL_SIZE_BYTES_KEY = "total_size_bytes"


def add_binary_size_output_property(result, source, binary_size):
  result.presentation.properties['binary_size_%s' % source] = binary_size


def RunSteps(api):
  api.vars.setup()

  checkout_root = api.checkout.default_checkout_root
  api.checkout.bot_update(checkout_root=checkout_root)

  out_dir = api.vars.swarming_out_dir
  # Any binaries to scan should be here.
  bin_dir = api.vars.build_dir

  api.file.ensure_directory('mkdirs out_dir', out_dir, mode=0o777)

  analyzed = 0
  with api.context(cwd=bin_dir):

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

    files = api.file.glob_paths(
        'find dm',
        bin_dir,
        'dm',
        test_data=['dm'])
    analyzed += len(files)
    if files:
      make_treemap(api, checkout_root, out_dir, files)

  if not analyzed: # pragma: nocover
    raise Exception('No files were analyzed!')


def keys_and_props(api):
  keys = []
  for k in sorted(api.vars.builder_cfg.keys()):
      if not k in ['role']:
        keys.extend([k, api.vars.builder_cfg[k]])
  keystr = ' '.join(keys)

  props = [
    'gitHash', api.properties['revision'],
    'swarming_bot_id', api.vars.swarming_bot_id,
    'swarming_task_id', api.vars.swarming_task_id,
  ]

  if api.vars.is_trybot:
    props.extend([
      'issue',    api.vars.issue,
      'patchset', api.vars.patchset,
      'patch_storage', api.vars.patch_storage,
    ])
  propstr = ' '.join(str(prop) for prop in props)
  return (keystr, propstr)


# Get the raw size and a few metrics from bloaty
def analyze_cpp_lib(api, checkout_root, out_dir, files):
  (keystr, propstr) = keys_and_props(api)
  bloaty_exe = api.path.start_dir.joinpath('bloaty', 'bloaty')

  for f in files:
    skia_dir = checkout_root.joinpath('skia')
    with api.context(cwd=skia_dir):
      script = skia_dir.joinpath('infra', 'bots', 'buildstats',
                                 'buildstats_cpp.py')
      step_data = api.run(api.step, 'Analyze %s' % f,
          cmd=['python3', script, f, out_dir, keystr, propstr, bloaty_exe,
               TOTAL_SIZE_BYTES_KEY, MAGIC_SEPERATOR],
          stdout=api.raw_io.output())
      if step_data and step_data.stdout:
        sections = step_data.stdout.decode('utf-8').split(MAGIC_SEPERATOR)
        result = api.step.active_result
        logs = result.presentation.logs
        logs['perf_json'] = sections[2].split('\n')

        add_binary_size_output_property(result, api.path.basename(f), (
            ast.literal_eval(sections[2])
              .get('results', {})
              .get(api.path.basename(f), {})
              .get('default', {})
              .get(TOTAL_SIZE_BYTES_KEY, {})))


# make a zip file containing an HTML treemap of the files
def make_treemap(api, checkout_root, out_dir, files):
  for f in files:
    env = {'DOCKER_CONFIG': '/home/chrome-bot/.docker'}
    with api.env(env):
      skia_dir = checkout_root.joinpath('skia')
      with api.context(cwd=skia_dir):
        script = skia_dir.joinpath('infra', 'bots', 'buildstats',
                                   'make_treemap.py')
        api.run(api.step, 'Make code size treemap %s' % f,
                cmd=['python3', script, f, out_dir],
                stdout=api.raw_io.output())


def GenTests(api):
  builder = 'BuildStats-Ubuntu24.04-Clang-x86_64-Release'
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
    api.step_data('Analyze [START_DIR]/build/libskia.so',
        stdout=api.raw_io.output(sample_cpp))
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
    api.step_data('Analyze [START_DIR]/build/libskia.so',
        stdout=api.raw_io.output(sample_cpp))
  )

sample_cpp = """
#$%^&*
Report A
    Total size: 50 bytes
#$%^&*
{
  "some": "json",
  "results": {
    "libskia.so": {
      "default": {
        "total_size_bytes": 7391117,
        "gzip_size_bytes": 2884841
      }
    }
  }
}
"""
