# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for uploading calmbench results.


import calendar


DEPS = [
  'flavor',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'recipe_engine/time',
  'run',
  'vars',
]


def FindFile(api, suffix):
  with api.context(cwd=api.path['start_dir']):
    results = api.file.glob_paths(
        'find %s results' % suffix,
        api.path['start_dir'].join('perf'),
        '*.%s' % suffix,
        test_data=['bench_modified_master.%s' % suffix])
  if len(results) != 1:  # pragma: nocover
    raise Exception('Unable to find the %s file!' % suffix)
  return results[0]


def RunSteps(api):
  api.vars.setup()
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)
  api.flavor.setup()

  builder_name = api.properties['buildername']

  now = api.time.utcnow()

  json_src = FindFile(api, "json")
  csv_src = FindFile(api, "csv")

  ts = int(calendar.timegm(now.utctimetuple()))
  basename = "bench_modified_master_%s_%d" % (api.properties['revision'], ts)

  gs_path = '/'.join((
      'calmbench-v1', str(now.year).zfill(4),
      str(now.month).zfill(2), str(now.day).zfill(2), str(now.hour).zfill(2),
      builder_name))

  issue = api.properties.get('patch_issue')
  patchset = api.properties.get('patch_set')
  if issue and patchset:
    gs_path = '/'.join(('trybot', gs_path, str(issue), str(patchset)))

  dst = '/'.join((
      'gs://%s' % api.properties['gs_bucket'], gs_path, basename))

  json_dst = dst + ".json"
  csv_dst = dst + ".csv"

  api.step(
      'upload json',
      cmd=['gsutil', 'cp', '-z', 'json', json_src, json_dst],
      infra_step=True)
  api.step(
      'upload csv',
      cmd=['gsutil', 'cp', '-z', 'csv', csv_src, csv_dst],
      infra_step=True)


def GenTests(api):
  builder = 'Calmbench-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All'
  yield (
    api.test('normal_bot') +
    api.properties(buildername=builder,
                   repository='https://skia.googlesource.com/skia.git',
                   gs_bucket='skia-calmbench',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   revision='abc123',
                   path_config='kitchen')
  )

  yield (
    api.test('trybot') +
    api.properties(buildername=builder,
                   repository='https://skia.googlesource.com/skia.git',
                   gs_bucket='skia-calmbench',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   revision='abc123',
                   path_config='kitchen',
                   patch_storage='gerrit') +
    api.properties.tryserver(
        buildername=builder,
        gerrit_project='skia',
        gerrit_url='https://skia-review.googlesource.com/',
    )
  )
