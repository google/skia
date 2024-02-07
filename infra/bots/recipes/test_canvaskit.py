# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which runs the Canvaskit tests using docker

PYTHON_VERSION_COMPATIBILITY = "PY3"

DEPS = [
  'checkout',
  'docker',
  'env',
  'flavor',
  'infra',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'gold_upload',
  'run',
  'vars',
]


DOCKER_IMAGE = 'gcr.io/skia-public/gold-karma-chrome-tests:87.0.4280.88_v2'
INNER_KARMA_SCRIPT = 'skia/infra/canvaskit/test_canvaskit.sh'

def RunSteps(api):
  api.vars.setup()
  api.flavor.setup('dm')
  checkout_root = api.path['start_dir']
  out_dir = api.vars.swarming_out_dir

  # The karma script is configured to look in ./build/ for
  # the test files to load, so we must copy them there (see Set up for docker).
  copy_dest = checkout_root.join('skia', 'modules', 'canvaskit',
                                 'build')
  api.file.ensure_directory('mkdirs copy_dest', copy_dest, mode=0o777)
  base_dir = api.vars.build_dir
  copies = [
    {
      'src': base_dir.join('canvaskit.js'),
      'dst': copy_dest.join('canvaskit.js'),
    },
    {
      'src': base_dir.join('canvaskit.wasm'),
      'dst': copy_dest.join('canvaskit.wasm'),
    },
  ]
  recursive_read = [checkout_root.join('skia')]

  args = [
    '--builder',              api.vars.builder_name,
    '--git_hash',             api.properties['revision'],
    '--buildbucket_build_id', api.properties.get('buildbucket_build_id', ''),
    '--browser',              'Chrome',
    '--config',               api.vars.configuration,
    '--source_type',          'canvaskit',
  ]
  if api.vars.is_trybot:
    args.extend([
      '--issue',         api.vars.issue,
      '--patchset',      api.vars.patchset,
    ])

  api.docker.run(
      name='Test CanvasKit with Docker',
      docker_image=DOCKER_IMAGE,
      src_dir=checkout_root,
      out_dir=out_dir,
      script=checkout_root.join(INNER_KARMA_SCRIPT),
      args=args,
      docker_args=None,
      copies=copies,
      recursive_read=recursive_read,
      attempts=3,
  )

  api.gold_upload.upload()

def GenTests(api):
  yield (
      api.test('Test-Debian10-EMCC-GCE-GPU-WEBGL1-wasm-Debug-All-CanvasKit') +
      api.properties(buildername=('Test-Debian10-EMCC-GCE-GPU-WEBGL1'
                                  '-wasm-Debug-All-CanvasKit'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     gs_bucket='skia-infra-gm',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('canvaskit_trybot') +
      api.properties(buildername=('Test-Debian10-EMCC-GCE-CPU-AVX2'
                                  '-wasm-Debug-All-CanvasKit'),
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
