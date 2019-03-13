# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which runs the PathKit tests using docker

import os

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
    # 'gitHash', api.properties['revision'],
    # 'swarming_bot_id', api.vars.swarming_bot_id,
    # 'swarming_task_id', api.vars.swarming_task_id,
  ]
  # Try adding builder if key is not populated.
  if api.vars.is_trybot:
    properties.extend([
      'issue',  api.vars.issue,
      'patchset',  api.vars.patchset,
      'patch_storage', api.vars.patch_storage,
    ])

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

  # Need some way to extract the gitHash, swarming_bot_id, swarming_task_id and
  # key from DM.json...

  # Run DM on each lottie file and parse the trace files.
  for lottie_file in api.file.listdir(
      'list lottie files', api.flavor.device_dirs.lotties_dir,
      test_data=['lottie1.json', 'lottie2.json']):
    lottie_filename = os.path.basename(api.path.abspath(lottie_file))
    trace_file = api.flavor.device_dirs.dm_dir.join('trace-%s' % lottie_filename)
    args.extend(['--trace', trace_file])
    args.extend(['--match', lottie_filename])
    # Lets see what happens with the below!
    args.extend(['--writePath', api.flavor.device_dirs.dm_dir])
    api.run(api.flavor.step, 'dm', cmd=args, abort_on_failure=False)

  # with api.tempfile.temp_dir('dm_trace_outputs') as local_tmp:
  perf_results = {}
  for lottie_file in api.file.listdir(
      'list lottie files', api.flavor.device_dirs.lotties_dir,
      test_data=['lottie1.json', 'lottie2.json']):
    lottie_filename = os.path.basename(api.path.abspath(lottie_file))
    trace_file = api.flavor.host_dirs.dm_dir.join('trace-%s' % lottie_filename)
    # wanted this to be local_tmp..
    # perf_data_dir is the goal in the end???
    api.flavor.copy_directory_contents_to_host(api.flavor.device_dirs.dm_dir, api.flavor.host_dirs.dm_dir)

    # Read the trace file now!
    test_data = api.properties.get('trace_test_data', '{}')
    content = api.file.read_text('Read %s' % trace_file,
                                 trace_file, test_data=test_data)
    out = api.json.loads(content)
    print 'Trace file contents of %s are:' % lottie_filename
    print out

    # Skip the first "skottie::Animation::seek" constructor call.
    skipped_first_seek = False
    lottie_file_results = {}
    encountered_renders = 0
    encountered_seeks = 0
    perf_results[lottie_filename] = lottie_file_results
    for trace in out:
      print 'Processing trace: %s' % trace
      if 'skottie::Animation::seek' in trace['name']:
        if not skipped_first_seek:
          skipped_first_seek = True
          continue
        print 'PASS 1'
        encountered_seeks += 1
        calculate_perf_metrics('seek', encountered_seeks, trace['dur'],
                               lottie_file_results)
      elif 'skottie::Animation::render' in trace['name']:
        print 'PASS 2'
        encountered_renders += 1
        calculate_perf_metrics('render', encountered_renders, trace['dur'],
                               lottie_file_results)
      else:
        print 'CONTINUE 1'
        # We do not recognize this trace.
        continue
      duration = trace['dur']

      print 'something?'
      print trace['name']
      print trace['dur']
      print trace
    # print ' OUTPUT OUTPU'
    # print test_data
    print out
    # asdfa


  print 'these are args!'
  print args
  print perf_results
  # Temperary below just for testing!
  assert perf_results['lottie1.json']['frame_max'] == 216
  assert perf_results['lottie1.json']['frame_min'] == 140
  assert perf_results['lottie1.json']['frame_avg'] == 178

  assert perf_results['lottie1.json']['seek_max'] == 2.25
  assert perf_results['lottie1.json']['seek_min'] == 1.17
  assert perf_results['lottie1.json']['seek_avg'] == 1.71
  assert perf_results['lottie1.json']['render_max'] == 216
  assert perf_results['lottie1.json']['render_min'] == 140
  assert perf_results['lottie1.json']['render_avg'] == 178
  print perf_results

  # How do I keep these in a perf.json file?
  # Read DM.json and get everything out of it!
  dm_json = api.flavor.host_dirs.dm_dir.join('dm.json')
  dm_json_test_data = api.properties.get('dm_json_test_data', '{}')
  dm_content = api.file.read_text('Read dm.json', dm_json, test_data=dm_json_test_data)
  dm_json = api.json.loads(dm_content)

  # Add necessary things to it and then output to a new perf.json.
  perf_json = {
      'gitHash': api.properties['revision'],
      'swarming_bot_id': api.vars.swarming_bot_id,
      'swarming_task_id': api.vars.swarming_task_id,
  }
  if api.vars.is_trybot:
    perf_json['issue'] = api.vars.issue
    perf_json['patchset'] = api.vars.patchset
    perf_json['patch_storage'] = api.vars.patch_storage
  # Add the key.
  print 'THIS IS THE KEY'
  print dm_json['key']
  print perf_json
  perf_json['key'] = dm_json['key']
  perf_json['key']['bench_type'] = 'tracing'
  perf_json['key']['source_type'] = 'skottie'
  # Add the tracing results.
  perf_json['results'] = {}
  perf_json['results']['gles'] = perf_results

  print 'final perf_json is this:'
  print perf_json
  asdf

  # Put everything in api.flavor.host_dirs.dm_dir at the end and call it
  # perf.json or whatever upload-perf expects

  # Grab the trace file and convert it to a perf file..

  # Copy results to swarming out dir.
  api.file.ensure_directory(
      'makedirs perf_dir',
      api.flavor.host_dirs.perf_data_dir)
  api.flavor.copy_directory_contents_to_host(
      api.flavor.device_dirs.perf_data_dir,
      api.flavor.host_dirs.perf_data_dir)

  # Do I need to clean up anything from the thingy?
  # Upload step does the actual upload?
  pass



def calculate_perf_metrics(metric, times_encountered, duration, results):
  metric_max = '%s_max' % metric
  results[metric_max] = (
    duration if duration > results.get(metric_max, 0)
    else results.get(metric_max, 0))

  metric_min = '%s_min' % metric
  results[metric_min] = (
    duration if duration < results.get(metric_min, 0)
    else results.get(metric_min, duration))

  metric_avg = '%s_avg' % metric
  results[metric_avg] = (
      duration + results.get(metric_avg, 0))/times_encountered


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
[{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":452,"dur":2.57,"tid":1,"pid":0},{"ph":"X","name":"void SkCanvas::drawPaint(const SkPaint &)","ts":473,"dur":2.67e+03,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":3.15e+03,"dur":2.25,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::render(SkCanvas *, const SkRect *, RenderFlags) const","ts":3.15e+03,"dur":216,"tid":1,"pid":0},{"ph":"X","name":"void SkCanvas::drawPath(const SkPath &, const SkPaint &)","ts":3.35e+03,"dur":15.1,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":3.37e+03,"dur":1.17,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::render(SkCanvas *, const SkRect *, RenderFlags) const","ts":3.37e+03,"dur":140,"tid":1,"pid":0}]
"""
  dm_json_test_data = """
{
  "gitHash": "bac53f089dbc473862bc5a2e328ba7600e0ed9c4",
  "swarming_bot_id": "skia-rpi-094",
  "swarming_task_id": "438f11c0e19eab11",
  "key": {
    "arch": "arm",
    "compiler": "Clang",
    "cpu_or_gpu": "GPU",
    "cpu_or_gpu_value": "Mali400MP2",
    "extra_config": "Android",
    "model": "AndroidOne",
    "os": "Android"
   },
   "results": {
   }
}
"""
  yield (
      api.test('Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-AndroidSkottieTracing') +
      api.properties(buildername=('Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-AndroidSkottieTracing'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     task_id='abc123',
                     trace_test_data=trace_output,
                     dm_json_test_data=dm_json_test_data,
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
  return
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
