# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for uploading nanobench results.

PYTHON_VERSION_COMPATIBILITY = "PY2+3"

DEPS = [
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'recipe_engine/time',
  'vars',
]


def RunSteps(api):
  # Upload the nanobench results.
  api.vars.setup()

  now = api.time.utcnow()
  src_path = api.path['start_dir'].join('perf')
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
      api.vars.builder_name))

  if api.vars.is_trybot:
    gs_path = '/'.join(('trybot', gs_path,
                        str(api.vars.issue), str(api.vars.patchset)))

  dst = '/'.join((
      'gs://%s' % api.properties['gs_bucket'], gs_path, basename))

  api.step(
      'upload',
      cmd=['gsutil', 'cp', '-z', 'json', src, dst],
      infra_step=True)


def GenTests(api):
  builder = 'Perf-Debian10-Clang-GCE-CPU-AVX2-x86_64-All-Debug'
  yield (
    api.test('normal_bot') +
    api.properties(buildername=builder,
                   gs_bucket='skia-perf',
                   revision='abc123',
                   path_config='kitchen')
  )

  yield (
    api.test('trybot') +
    api.properties(buildername=builder,
                   gs_bucket='skia-perf',
                   revision='abc123',
                   path_config='kitchen') +
    api.properties.tryserver(
        buildername=builder,
        gerrit_project='skia',
        gerrit_url='https://skia-review.googlesource.com/',
    )
  )
