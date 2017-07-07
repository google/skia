# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for uploading DM results.


import calendar


DEPS = [
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'recipe_engine/time',
]


DM_JSON = 'dm.json'
UPLOAD_ATTEMPTS = 5
VERBOSE_LOG = 'verbose.log'


def cp(api, name, src, dst, extra_args=None):
  cmd = ['gsutil', 'cp']
  if extra_args:
    cmd.extend(extra_args)
  cmd.extend([src, dst])

  name = 'upload %s' % name
  for i in xrange(UPLOAD_ATTEMPTS):
    step_name = name
    if i > 0:
      step_name += ' (attempt %d)' % (i+1)
    try:
      api.step(step_name, cmd=cmd)
      break
    except api.step.StepFailure:
      if i == UPLOAD_ATTEMPTS - 1:
        raise


def RunSteps(api):
  builder_name = api.properties['buildername']
  revision = api.properties['revision']

  results_dir = api.path['start_dir'].join('dm')

  # Move dm.json and verbose.log to their own directory.
  json_file = results_dir.join(DM_JSON)
  log_file = results_dir.join(VERBOSE_LOG)
  tmp_dir = api.path['start_dir'].join('tmp_upload')
  api.file.ensure_directory('makedirs tmp dir', tmp_dir)
  api.file.copy('copy dm.json', json_file, tmp_dir)
  api.file.copy('copy verbose.log', log_file, tmp_dir)
  api.file.remove('rm old dm.json', json_file)
  api.file.remove('rm old verbose.log', log_file)

  # Upload the images.
  image_dest_path = 'gs://%s/dm-images-v1' % api.properties['gs_bucket']
  for ext in ['.png', '.pdf']:
    files_to_upload = api.file.glob_paths(
        'find images',
        results_dir,
        '*%s' % ext,
        test_data=['someimage.png'])
    # For some reason, glob returns results_dir when it should return nothing.
    files_to_upload = [f for f in files_to_upload if str(f).endswith(ext)]
    if len(files_to_upload) > 0:
      cp(api, 'images', results_dir.join('*%s' % ext), image_dest_path)

  # Upload the JSON summary and verbose.log.
  now = api.time.utcnow()
  summary_dest_path = '/'.join([
      'dm-json-v1',
      str(now.year ).zfill(4),
      str(now.month).zfill(2),
      str(now.day  ).zfill(2),
      str(now.hour ).zfill(2),
      revision,
      builder_name,
      str(int(calendar.timegm(now.utctimetuple())))])

  # Trybot results are further siloed by issue/patchset.
  issue = api.properties.get('patch_issue')
  patchset = api.properties.get('patch_set')
  if issue and patchset:
    summary_dest_path = '/'.join((
        'trybot', summary_dest_path, str(issue), str(patchset)))

  summary_dest_path = 'gs://%s/%s' % (api.properties['gs_bucket'],
                                      summary_dest_path)

  cp(api, 'JSON and logs', tmp_dir.join('*'), summary_dest_path,
     ['-z', 'json,log'])


def GenTests(api):
  builder = 'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug'
  yield (
    api.test('normal_bot') +
    api.properties(buildername=builder,
                   gs_bucket='skia-infra-gm',
                   revision='abc123',
                   path_config='kitchen')
  )

  yield (
    api.test('failed_once') +
    api.properties(buildername=builder,
                   gs_bucket='skia-infra-gm',
                   revision='abc123',
                   path_config='kitchen') +
    api.step_data('upload images', retcode=1)
  )

  yield (
    api.test('failed_all') +
    api.properties(buildername=builder,
                   gs_bucket='skia-infra-gm',
                   revision='abc123',
                   path_config='kitchen') +
    api.step_data('upload images', retcode=1) +
    api.step_data('upload images (attempt 2)', retcode=1) +
    api.step_data('upload images (attempt 3)', retcode=1) +
    api.step_data('upload images (attempt 4)', retcode=1) +
    api.step_data('upload images (attempt 5)', retcode=1)
  )

  builder = 'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug'
  yield (
      api.test('trybot') +
      api.properties(
          buildername=builder,
          gs_bucket='skia-infra-gm',
          revision='abc123',
          path_config='kitchen',
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=builder,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )
  )
