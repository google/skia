# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for uploading nanobench results.


DEPS = [
  'build/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'recipe_engine/time',
]


def RunSteps(api):
  # Upload the nanobench resuls.
  builder_name = api.properties['buildername']

  now = api.time.utcnow()
  src_path = api.path['cwd'].join(
      'perfdata', builder_name, 'data')
  results = api.file.glob(
      'find results',
      '*.json',
      cwd=src_path,
      test_data=['nanobench_abc123.json'],
      infra_step=True)
  if len(results) != 1:  # pragma: nocover
    raise Exception('Unable to find nanobench or skpbench JSON file!')

  src = src_path.join(results[0])
  basename = api.path.basename(src)
  gs_path = '/'.join((
      'nano-json-v1', str(now.year).zfill(4),
      str(now.month).zfill(2), str(now.day).zfill(2), str(now.hour).zfill(2),
      builder_name))

  issue = str(api.properties.get('issue', ''))
  patchset = str(api.properties.get('patchset', ''))
  if api.properties.get('patch_storage', '') == 'gerrit':
    issue = str(api.properties['patch_issue'])
    patchset = str(api.properties['patch_set'])
  if issue and patchset:
    gs_path = '/'.join(('trybot', gs_path, issue, patchset))

  dst = '/'.join(('gs://skia-perf', gs_path, basename))

  api.step('upload',
           cmd=['gsutil', 'cp', '-a', 'public-read', '-z', 'json', src, dst],
           infra_step=True)


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

  yield (
      api.test('recipe_with_gerrit_patch') +
      api.properties(
          buildername=builder,
          revision='abc123',
          path_config='kitchen',
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=builder,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )
  )
