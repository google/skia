# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which analyzes a compiled binary for information (e.g. file size)

DEPS = [
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'vars',
]

def RunSteps(api):
  api.vars.setup()
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
      analyze_web_file(api, out_dir, files)
  with api.context(cwd=bin_dir):
    files = api.file.glob_paths(
        'find JS files',
        bin_dir,
        '*.js',
        test_data=['pathkit.js'])
    if len(files):
      analyze_web_file(api, out_dir, files)


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
def analyze_web_file(api, out_dir, files):
  (keystr, propstr) = keys_and_props(api)

  for f in files:
    api.python.inline(
      name='Analyze %s' % f,
      program='''
import json
import os
import subprocess
import sys

input_file = sys.argv[1]
out_dir = sys.argv[2]
keystr = sys.argv[3]
propstr = sys.argv[4]

results = {
  'keys': { },
  'results': { }
}

props = propstr.split(' ')
for i in range(0, len(props), 2):
  results[props[i]] = props[i+1]

keys = keystr.split(' ')
for i in range(0, len(keys), 2):
  results['keys'][keys[i]] = keys[i+1]

r = {
  'total_size_bytes': os.path.getsize(input_file)
}

# Make a copy to avoid destroying the hardlinked file.
# Swarming hardlinks in the builds from isolated cache.
temp_file = input_file + '_tmp'
subprocess.check_call(['cp', input_file, temp_file])
subprocess.check_call(['gzip', temp_file])

r['gzip_size_bytes'] = os.path.getsize(temp_file + '.gz')

name = os.path.basename(input_file)

results['results'][name] = {
  # We need this top level layer 'config'/slice
  # Other analysis methods (e.g. libskia) might have
  # slices for data on the 'code' section, etc.
  'default' : r,
}

print json.dumps(results, indent=2)

with open(os.path.join(out_dir, name+'.json'), 'w') as output:
  output.write(json.dumps(results, indent=2))
''',
      args=[f, out_dir, keystr, propstr],
      infra_step=True)

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
