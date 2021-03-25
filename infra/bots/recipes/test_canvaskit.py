# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which runs the Canvaskit tests using docker

import calendar

DEPS = [
  'checkout',
  'docker',
  'env',
  'infra',
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
  'recipe_engine/time',
  'gsutil',
  'run',
  'vars',
]


DOCKER_IMAGE = 'gcr.io/skia-public/gold-karma-chrome-tests:87.0.4280.88_v1'
INNER_KARMA_SCRIPT = 'skia/infra/canvaskit/test_canvaskit.sh'
DM_JSON = 'dm.json'

def upload(api):
  revision = api.properties['revision']
  results_dir = api.vars.swarming_out_dir

  # Upload the images. It is preferred that the images are uploaded first
  # so they exist whenever the json is processed.
  image_dest_path = 'gs://%s/dm-images-v1' % api.properties['gs_bucket']
  for ext in ['.png']:
    files_to_upload = api.file.glob_paths(
        'find %s images' % ext,
        results_dir,
        '*%s' % ext,
        test_data=['someimage.png'])
    # For some reason, glob returns results_dir when it should return nothing.
    files_to_upload = [f for f in files_to_upload if str(f).endswith(ext)]
    if len(files_to_upload) > 0:
      api.gsutil.cp('%s images' % ext, results_dir.join('*%s' % ext),
                       image_dest_path, multithread=True)

  summary_dest_path = 'gs://%s' % api.properties['gs_bucket']
  ref = revision
  # Trybot results are siloed by issue/patchset.
  if api.vars.is_trybot:
    summary_dest_path = '/'.join([summary_dest_path, 'trybot'])
    ref = '%s_%s' % (str(api.vars.issue), str(api.vars.patchset))

  # Compute the directory to upload results to
  now = api.time.utcnow()
  summary_dest_path = '/'.join([
      summary_dest_path,
      'dm-json-v1',
      str(now.year ).zfill(4),
      str(now.month).zfill(2),
      str(now.day  ).zfill(2),
      str(now.hour ).zfill(2),
      ref,
      api.vars.builder_name,
      str(int(calendar.timegm(now.utctimetuple())))])

  # Directly upload dm.json if it exists.
  json_file = results_dir.join(DM_JSON)
  # -Z compresses the json file at rest with gzip.
  api.gsutil.cp('dm.json', json_file,
                summary_dest_path + '/' + DM_JSON, extra_args=['-Z'])

def RunSteps(api):
  api.vars.setup()
  checkout_root = api.path['start_dir']
  out_dir = api.vars.swarming_out_dir

  # The karma script is configured to look in ./npm_build/bin/ for
  # the test files to load, so we must copy them there (see Set up for docker).
  copy_dest = checkout_root.join('skia', 'modules', 'canvaskit',
                                 'npm_build', 'bin')
  api.file.ensure_directory('mkdirs copy_dest', copy_dest, mode=0o777)
  base_dir = api.vars.build_dir
  copies = {
    base_dir.join('canvaskit.js'): copy_dest.join('canvaskit.js'),
    base_dir.join('canvaskit.wasm'):    copy_dest.join('canvaskit.wasm'),
  }
  recursive_read = [checkout_root.join('skia')]

  args = [
    '--builder',              api.vars.builder_name,
    '--git_hash',             api.properties['revision'],
    '--buildbucket_build_id', api.properties.get('buildbucket_build_id', ''),
    '--bot_id',               api.vars.swarming_bot_id,
    '--task_id',              api.vars.swarming_task_id,
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

  upload(api)

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
