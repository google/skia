# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which runs the PathKit tests using docker

DEPS = [
  'env',
  'flavor',
  'infra',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
  'recipe_engine/tempfile',
  'run',
  'vars',
]


def perf_steps(api):
  """Run DM on lottie files with tracing turned on"""
  api.flavor.create_clean_device_dir(
        api.flavor.device_dirs.perf_data_dir)

  # Run DM.
  properties = [
    'gitHash',              api.properties['revision'],
    'builder',              api.vars.builder_name,
    'buildbucket_build_id', api.properties.get('buildbucket_build_id', ''),
    'task_id',              api.properties['task_id'],
  ]
  if api.vars.is_trybot:
    properties.extend([
      'issue',         api.vars.issue,
      'patchset',      api.vars.patchset,
      'patch_storage', api.vars.patch_storage,
    ])
  properties.extend(['swarming_bot_id', api.vars.swarming_bot_id])
  properties.extend(['swarming_task_id', api.vars.swarming_task_id])

  # See go/skottie-tracing for how these flags were selected.
  args = [
    'dm',
    '--resourcePath', api.flavor.device_dirs.resource_dir,
    '--lotties', api.flavor.device_dirs.lotties_dir,
    '--nocpu',
    '--config', 'gles',
    '--src', 'lottie',
    '--nonativeFonts',
    '--verbose',
    '--properties',
  ] + properties

  # Run DM on each lottie file and parse the trace files.
  with api.tempfile.temp_dir('trace_outputs') as tmp:
    for lottie_file in api.file.listdir(
        'list lottie files', api.flavor.device_dirs.lotties_dir,
        test_data=['lottie1.json', 'lottie2.json']):
      trace_file = tmp.join('trace-%s' % lottie_file)
      args.extend(['--trace', trace_file])
      args.extend(['--match', lottie_file])

      api.run(api.flavor.step, 'dm', cmd=args,
              abort_on_failure=False)

      # Read the trace file now!
      test_data = api.properties.get('trace_test_data', '{}')
      content = api.file.read_text('Read %s' % trace_file,
                                   trace_file, test_data=test_data)
      out = api.json.loads(content)
      print ' OUTPUT OUTPU'
      print out


  print 'these are args!'
  print args

  # Need to add --trace some temp fir
  # Need to add --match wow.json (when looping through all files!)

  # Run DM once for all lottie files.

  # Grab the trace file and convert it to a perf file..

  # Copy results to swarming out dir.
  api.file.ensure_directory(
      'makedirs perf_dir',
      api.flavor.host_dirs.perf_data_dir)
  api.flavor.copy_directory_contents_to_host(
      api.flavor.device_dirs.perf_data_dir,
      api.flavor.host_dirs.perf_data_dir)

  # Upload step does the actual upload?
  pass


def RunSteps(api):
  api.vars.setup()
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)
  api.flavor.setup()

  with api.context():
    try:
      api.flavor.install(resources=True, lotties=True)
      perf_steps(api)
    finally:
      api.flavor.cleanup_steps()
    api.run.check_failure()


TEST_BUILDERS = [
  'Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-AndroidSkottieTracing',
]


def GenTests(api):
  trace_output = """
[{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":452,"dur":2.57,"tid":1,"pid":0},{"ph":"X","name":"void SkCanvas::drawPaint(const SkPaint &)","ts":473,"dur":2.67e+03,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":3.15e+03,"dur":2.25,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::render(SkCanvas *, const SkRect *, RenderFlags) const","ts":3.15e+03,"dur":216,"tid":1,"pid":0}]
"""
  yield (
      api.test('Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-AndroidSkottieTracing') +
      api.properties(buildername=('Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-AndroidSkottieTracing'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     task_id='abc123',
                     trace_test_data=trace_output,
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('skottietracing_trybot') +
      api.properties(buildername=('Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-AndroidSkottieTracing'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     task_id='abc123',
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
