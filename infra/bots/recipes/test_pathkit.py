# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which runs the PathKit tests using docker

DEPS = [
  'checkout',
  'docker',
  'env',
  'flavor',
  'gold_upload',
  'infra',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
  'run',
  'vars',
]


DOCKER_IMAGE = 'gcr.io/skia-public/gold-karma-chrome-tests:87.0.4280.88_v2'
INNER_KARMA_SCRIPT = 'skia/infra/pathkit/test_pathkit.sh'


def RunSteps(api):
  api.vars.setup()
  api.flavor.setup("dm")
  checkout_root = api.path['start_dir']
  out_dir = api.vars.swarming_out_dir

  # The karma script is configured to look in ./npm-(asmjs|wasm)/bin/test/ for
  # the test files to load, so we must copy them there (see Set up for docker).
  copy_dest = checkout_root.join('skia', 'modules', 'pathkit',
                                 'npm-wasm', 'bin', 'test')
  if 'asmjs' in api.vars.builder_name:
    copy_dest = checkout_root.join('skia', 'modules', 'pathkit',
                                   'npm-asmjs', 'bin', 'test')

  base_dir = api.vars.build_dir
  bundle_name = 'pathkit.wasm'
  if 'asmjs' in api.vars.builder_name:
    # release mode has a .js.mem file that needs to come with.
    # debug mode has an optional .map file, but we can omit that for tests
    if 'Debug' in api.vars.builder_name:
      bundle_name = ''
    else:
      bundle_name = 'pathkit.js.mem'

  copies = {
    base_dir.join('pathkit.js'): copy_dest.join('pathkit.js'),
  }
  if bundle_name:
    copies[base_dir.join(bundle_name)] = copy_dest.join(bundle_name)
  recursive_read = [checkout_root.join('skia')]

  docker_args = None
  if 'asmjs' in api.vars.builder_name:
    docker_args = ['--env', 'ASM_JS=1']

  args = [
    '--builder',              api.vars.builder_name,
    '--git_hash',             api.properties['revision'],
    '--buildbucket_build_id', api.properties.get('buildbucket_build_id', ''),
    '--browser',              'Chrome',
    '--config',               api.vars.configuration,
    '--source_type',          'pathkit',
  ]
  if 'asmjs' in api.vars.builder_name:
    args.extend(['--compiled_language', 'asmjs']) # the default is wasm
  if api.vars.is_trybot:
    args.extend([
      '--issue',         api.vars.issue,
      '--patchset',      api.vars.patchset,
    ])

  api.docker.run(
      name='Test PathKit with Docker',
      docker_image=DOCKER_IMAGE,
      src_dir=checkout_root,
      out_dir=out_dir,
      script=checkout_root.join(INNER_KARMA_SCRIPT),
      args=args,
      docker_args=docker_args,
      copies=copies,
      recursive_read=recursive_read,
      attempts=3,
  )

  api.gold_upload.upload()


def GenTests(api):
  yield (
      api.test('Test-Debian10-EMCC-GCE-CPU-AVX2-wasm-Debug-All-PathKit') +
      api.properties(buildername=('Test-Debian10-EMCC-GCE-CPU-AVX2'
                                  '-wasm-Debug-All-PathKit'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     gs_bucket='skia-infra-gm',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('Test-Debian10-EMCC-GCE-CPU-AVX2-asmjs-Debug-All-PathKit') +
      api.properties(buildername=('Test-Debian10-EMCC-GCE-CPU-AVX2'
                                  '-asmjs-Debug-All-PathKit'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     gs_bucket='skia-infra-gm',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('Test-Debian10-EMCC-GCE-CPU-AVX2-asmjs-Release-All-PathKit') +
      api.properties(buildername=('Test-Debian10-EMCC-GCE-CPU-AVX2'
                                  '-asmjs-Release-All-PathKit'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     gs_bucket='skia-infra-gm',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('pathkit_trybot') +
      api.properties(buildername=('Test-Debian10-EMCC-GCE-CPU-AVX2'
                                  '-wasm-Debug-All-PathKit'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     gs_bucket='skia-infra-gm',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     patch_ref='89/456789/12',
                     patch_repo='https://skia.googlesource.com/skia.git',
                     patch_storage='gerrit',
                     patch_set=7,
                     patch_issue=1234,
                     gerrit_project='skia',
                     gerrit_url='https://skia-review.googlesource.com/')
  )
