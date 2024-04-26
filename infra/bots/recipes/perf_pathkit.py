# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which runs the PathKit tests using docker

PYTHON_VERSION_COMPATIBILITY = "PY3"

DEPS = [
  'checkout',
  'docker',
  'env',
  'infra',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'run',
  'vars',
]


DOCKER_IMAGE = 'gcr.io/skia-public/perf-karma-chrome-tests:87.0.4280.88_v1'
INNER_KARMA_SCRIPT = 'skia/infra/pathkit/perf_pathkit.sh'


def RunSteps(api):
  api.vars.setup()
  checkout_root = api.path.start_dir
  out_dir = api.vars.swarming_out_dir

  # Make sure this exists, otherwise Docker will make it with root permissions.
  api.file.ensure_directory('mkdirs out_dir', out_dir, mode=0o777)

  # The karma script is configured to look in ./npm-(asmjs|wasm)/bin/ for
  # the test files to load, so we must copy them there (see Set up for docker).
  copy_dest = checkout_root.join('skia', 'modules', 'pathkit',
                                 'npm-wasm', 'bin')
  if 'asmjs' in api.vars.builder_name:
    copy_dest = checkout_root.join('skia', 'modules', 'pathkit',
                                   'npm-asmjs', 'bin')

  base_dir = api.vars.build_dir
  bundle_name = 'pathkit.wasm'
  if 'asmjs' in api.vars.builder_name:
    bundle_name = 'pathkit.js.mem'

  copies = [
    {
      'src': base_dir.join('pathkit.js'),
      'dst': copy_dest.join('pathkit.js'),
    },
    {
      'src': base_dir.join(bundle_name),
      'dst': copy_dest.join(bundle_name),
    },
  ]
  recursive_read = [checkout_root.join('skia')]

  docker_args = None
  if 'asmjs' in api.vars.builder_name:
    docker_args = ['--env', 'ASM_JS=1']

  args = [
    '--builder',              api.vars.builder_name,
    '--git_hash',             api.properties['revision'],
    '--buildbucket_build_id', api.properties.get('buildbucket_build_id', ''),
    '--bot_id',               api.vars.swarming_bot_id,
    '--task_id',              api.vars.swarming_task_id,
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
      name='Performance tests of PathKit with Docker',
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


def GenTests(api):
  yield (
      api.test('Perf-Debian10-EMCC-GCE-CPU-AVX2-wasm-Release-All-PathKit') +
      api.properties(buildername=('Perf-Debian10-EMCC-GCE-CPU-AVX2'
                                  '-wasm-Release-All-PathKit'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('Perf-Debian10-EMCC-GCE-CPU-AVX2-asmjs-Release-All-PathKit') +
      api.properties(buildername=('Perf-Debian10-EMCC-GCE-CPU-AVX2'
                                  '-asmjs-Release-All-PathKit'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('pathkit_trybot') +
      api.properties(buildername=('Perf-Debian10-EMCC-GCE-CPU-AVX2'
                                  '-wasm-Release-All-PathKit'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
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
