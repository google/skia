# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming perf.


import calendar
import json
import os


DEPS = [
  'env',
  'flavor',
  'gcloud',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/raw_io',
  'recipe_engine/service_account',
  'recipe_engine/step',
  'recipe_engine/time',
  'run',
  'vars',
]


def perf_steps(api):
  """Run Skia benchmarks."""
  do_upload = api.properties.get('do_upload') == 'true'
  images = api.properties.get('images') == 'true'
  resources = api.properties.get('resources') == 'true'
  skps = api.properties.get('skps') == 'true'
  svgs = api.properties.get('svgs') == 'true'
  texttraces = api.properties.get('texttraces') == 'true'

  api.flavor.install(
      resources=resources,
      skps=skps,
      images=images,
      svgs=svgs,
      texttraces=texttraces,
  )

  if do_upload:
    api.flavor.create_clean_device_dir(
        api.flavor.device_dirs.perf_data_dir)

  # Find nanobench flags.
  args = json.loads(api.properties['nanobench_flags'])
  props = json.loads(api.properties['nanobench_properties'])
  swarming_bot_id = api.vars.swarming_bot_id
  swarming_task_id = api.vars.swarming_task_id
  if do_upload:
    args.append('--properties')
    # Map iteration order is arbitrary; in order to maintain a consistent step
    # ordering, sort by key.
    for k in sorted(props.keys()):
      v = props[k]
      if v == '${SWARMING_BOT_ID}':
        v = swarming_bot_id
      elif v == '${SWARMING_TASK_ID}':
        v = swarming_task_id
      if v != '':
        args.extend([k, v])

  # Paths to required resources.
  if resources:
    args.extend(['-i', api.flavor.device_dirs.resource_dir])
  if skps:
    args.extend(['--skps', api.flavor.device_dirs.skp_dir]),
  if images:
    args.extend(['--images', api.flavor.device_path_join(
        api.flavor.device_dirs.images_dir, 'nanobench')])
  if texttraces:
    assert api.flavor.device_dirs.texttraces_dir
    args.extend(['--texttraces', api.flavor.device_dirs.texttraces_dir])
  if svgs:
    args.extend(['--svgs',  api.flavor.device_dirs.svg_dir])
  if do_upload:
    now = api.time.utcnow()
    ts = int(calendar.timegm(now.utctimetuple()))
    json_path = api.flavor.device_path_join(
        api.flavor.device_dirs.perf_data_dir,
        'nanobench_%s_%d.json' % (api.properties['revision'], ts))
    args.extend(['--outResultsFile', json_path])

  api.run(api.flavor.step, 'nanobench', cmd=args,
          abort_on_failure=False)

  if do_upload:
    # Copy results to swarming out dir.
    api.file.ensure_directory(
        'makedirs perf_dir',
        api.flavor.host_dirs.perf_data_dir)
    api.flavor.copy_directory_contents_to_host(
        api.flavor.device_dirs.perf_data_dir,
        api.flavor.host_dirs.perf_data_dir)

    # Upload results.
    results = api.file.glob_paths(
        'find results',
        api.flavor.host_dirs.perf_data_dir,
        '*.json',
        test_data=['nanobench_abc123.json'])
    results = [f for f in results if str(f).endswith('.json')]
    if len(results) != 1:  # pragma: nocover
      raise Exception('Unable to find nanobench JSON file!')

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
    api.gcloud.cp('upload %s' % basename, src, dst)


def RunSteps(api):
  api.vars.setup()
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)
  api.flavor.setup('nanobench')

  try:
    perf_steps(api)
  finally:
    api.flavor.cleanup_steps()
  api.run.check_failure()


TEST_BUILDERS = [
  'Perf-Android-Clang-Nexus7-CPU-Tegra3-arm-Debug-All-Android',
  'Perf-Win10-Clang-Golo-GPU-QuadroP400-x86_64-Release-All-ANGLE',
]


def GenTests(api):
  for builder in TEST_BUILDERS:
    props = dict(
      buildername=builder,
      gs_bucket='skia-perf',
      nanobench_flags='["nanobench","--example","--flags"]',
      nanobench_properties=('{"key1":"value1","key2":"",'
                            '"bot":"${SWARMING_BOT_ID}",'
                            '"task":"${SWARMING_TASK_ID}"}'),
      path_config='kitchen',
      resources='true',
      revision='abc123',
      swarm_out_dir='[SWARM_OUT_DIR]'
    )
    props['svgs'] = 'true'
    if 'Debug' not in builder:
      props['do_upload'] = 'true'
    if 'GPU' not in builder:
      props['images'] = 'true'
    if 'iOS' not in builder:
      props['skps'] = 'true'
    if 'Android' in builder and 'CPU' in builder:
      props['texttraces'] = 'true'
    test = (
      api.test(builder) +
      api.properties(**props) +
      api.path.exists(
          api.path.start_dir.joinpath('skia'),
          api.path.start_dir.joinpath('skia', 'infra', 'bots', 'assets',
                                      'skimage', 'VERSION'),
          api.path.start_dir.joinpath('skia', 'infra', 'bots', 'assets',
                                      'skp', 'VERSION'),
      ) +
      api.step_data('get swarming bot id',
          stdout=api.raw_io.output('skia-bot-123')) +
      api.step_data('get swarming task id',
          stdout=api.raw_io.output('123456'))
    )
    if 'Win' in builder:
      test += api.platform('win', 64)

    yield test

    if 'Debug' not in builder:
      # Also test trybot path.
      trybot_test = (
        api.test(builder + '_trybot') +
        api.properties(**props) +
        api.properties.tryserver(
            buildername=builder,
            gerrit_project='skia',
            gerrit_url='https://skia-review.googlesource.com/',
        ) +
        api.path.exists(
            api.path.start_dir.joinpath('skia'),
            api.path.start_dir.joinpath('skia', 'infra', 'bots', 'assets',
                                        'skimage', 'VERSION'),
            api.path.start_dir.joinpath('skia', 'infra', 'bots', 'assets',
                                        'skp', 'VERSION'),
        ) +
        api.step_data('get swarming bot id',
            stdout=api.raw_io.output('skia-bot-123')) +
        api.step_data('get swarming task id',
            stdout=api.raw_io.output('123456'))
      )
      if 'Win' in builder:
        trybot_test += api.platform('win', 64)
      yield trybot_test
