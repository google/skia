# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for uploading nanobench results.


DEPS = [
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'recipe_engine/time',
]


def RunSteps(api):
  # Upload the nanobench resuls.
  builder_name = api.properties['buildername']

  now = api.time.utcnow()
  src_path = api.path['start_dir'].join(
      'perfdata', builder_name, 'data')
  with api.context(cwd=src_path):
    results = api.file.glob_paths(
        'find results',
        src_path,
        '*.json',
        test_data=['nanobench_abc123.json'])
  if len(results) != 1:  # pragma: nocover
    raise Exception('Unable to find nanobench or skpbench JSON file!')

  src = results[0]
  basename = api.path.basename(src)
  gs_path = '/'.join((
      'nano-json-v1', str(now.year).zfill(4),
      str(now.month).zfill(2), str(now.day).zfill(2), str(now.hour).zfill(2),
      builder_name))

  issue = api.properties.get('patch_issue')
  patchset = api.properties.get('patch_set')
  if issue and patchset:
    gs_path = '/'.join(('trybot', gs_path, str(issue), str(patchset)))

  dst = '/'.join((
      'gs://%s' % api.properties['gs_bucket'], gs_path, basename))

  api.step(
      'upload',
      cmd=['gsutil', 'cp', '-z', 'json', src, dst],
      infra_step=True)


def GenTests(api):
  builder = 'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug'
  yield (
    api.test('normal_bot') +
    api.properties(buildername=builder,
                   gs_bucket='skia-perf',
                   revision='abc123',
                   path_config='kitchen')
  )

  builder = 'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug'
  yield (
    api.test('trybot') +
    api.properties(buildername=builder,
                   gs_bucket='skia-perf',
                   revision='abc123',
                   path_config='kitchen',
                   patch_storage='gerrit') +
    api.properties.tryserver(
        buildername=builder,
        gerrit_project='skia',
        gerrit_url='https://skia-review.googlesource.com/',
    )
  )
