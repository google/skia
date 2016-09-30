# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for uploading DM results.


DEPS = [
  'build/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/shutil',
  'recipe_engine/step',
  'recipe_engine/time',
]


import time


DM_JSON = 'dm.json'
GS_BUCKET = 'gs://skia-infra-gm'
VERBOSE_LOG = 'verbose.log'


def RunSteps(api):
  builder_name = api.properties['buildername']
  revision = api.properties['revision']

  patch_storage = api.properties.get('patch_storage', 'rietveld')
  issue = None
  patchset = None
  if builder_name.endswith('-Trybot'):
    if patch_storage == 'gerrit':
      issue = str(api.properties['event.change.number'])
      patchset = str(api.properties['event.patchSet.ref'].split('/')[-1])
    else:
      issue = str(api.properties['issue'])
      patchset = str(api.properties['patchset'])

  results_dir = api.path['cwd'].join('dm')

  # Move dm.json and verbose.log to their own directory.
  json_file = results_dir.join(DM_JSON)
  log_file = results_dir.join(VERBOSE_LOG)
  tmp_dir = api.path['cwd'].join('tmp_upload')
  api.shutil.makedirs('tmp dir', tmp_dir, infra_step=True)
  api.shutil.copy('copy dm.json', json_file, tmp_dir)
  api.shutil.copy('copy verbose.log', log_file, tmp_dir)
  api.shutil.remove('rm old dm.json', json_file)
  api.shutil.remove('rm old verbose.log', log_file)

  # Upload the images.
  image_dest_path = '/'.join((GS_BUCKET, 'dm-images-v1'))
  files_to_upload = api.file.glob(
      'find images',
      results_dir.join('*'),
      test_data=['someimage.png'],
      infra_step=True)
  if len(files_to_upload) > 0:
    api.step(
        'upload images',
        cmd=['gsutil', 'cp', results_dir.join('*'), image_dest_path],
     )

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
      str(int(time.mktime(now.utctimetuple())))])

  # Trybot results are further siloed by issue/patchset.
  if builder_name.endswith('-Trybot'):
    if not (issue and patchset):  # pragma: nocover
      raise Exception('issue and patchset properties are required for trybots.')
    summary_dest_path = '/'.join(('trybot', summary_dest_path, issue, patchset))

  summary_dest_path = '/'.join((GS_BUCKET, summary_dest_path))

  api.step(
      'upload JSON and logs',
      cmd=['gsutil', 'cp', '-z', 'json,log', tmp_dir.join('*'),
           summary_dest_path],
  )


def GenTests(api):
  builder = 'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug'
  yield (
    api.test('normal_bot') +
    api.properties(buildername=builder,
                   revision='abc123',
                   path_config='kitchen')
  )

  builder = 'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-Trybot'
  yield (
    api.test('trybot') +
    api.properties(buildername=builder,
                   revision='abc123',
                   path_config='kitchen',
                   issue='12345',
                   patchset='1002')
  )

  gerrit_kwargs = {
    'patch_storage': 'gerrit',
    'repository': 'skia',
    'event.patchSet.ref': 'refs/changes/00/2100/2',
    'event.change.number': '2100',
  }
  yield (
      api.test('recipe_with_gerrit_patch') +
      api.properties(
          buildername=builder,
          revision='abc123',
          path_config='kitchen',
          **gerrit_kwargs)
  )
