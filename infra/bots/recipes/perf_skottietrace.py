# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which runs DM with trace flag on Lottie files.
# Design doc: go/skottie-tracing


import os
import re


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


lottie_samples_to_skip = [
  # Skip because of consecutive seeks. fmalita@ is investigating.
  'gears.json',
]


def perf_steps(api):
  """Run DM on lottie files with tracing turned on"""
  api.flavor.create_clean_device_dir(
        api.flavor.device_dirs.dm_dir)
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
  ]
  lottie_files = api.file.listdir(
      'list lottie files', api.flavor.host_dirs.lotties_dir,
      test_data=['lottie1.json', 'lottie2.json', 'lottie 3.json',
                 'skip_lottie.json', 'LICENSE'])
  # Run DM on each lottie file and parse the trace files.
  push_dm = True
  test_skip_lotties = api.properties.get('test_skip_lotties', [])
  for lottie_file in lottie_files:
    lottie_filename = os.path.basename(api.path.abspath(lottie_file))
    if (not lottie_filename.endswith('.json') or
        lottie_filename in (test_skip_lotties or lottie_samples_to_skip)):
      continue
    trace_file = api.flavor.device_dirs.dm_dir + '/trace-%s' % lottie_filename
    # Sanitize the file name.
    # trace_file = trace_file.replace(' ', '_').replace('(', '').replace(')', '')
    args.extend(['--trace', '%s' % trace_file])
    args.extend(['--match', '%s' % lottie_filename])
    print 'RUNNING DM with:'
    print args
    api.run(api.flavor.step, 'dm', cmd=args, abort_on_failure=False,
            skip_push=not push_dm)
    # We already pushed the binary once. No need to waste time by pushing
    # the same binary repeatedly.
    push_dm = False

  perf_results = {}
  for lottie_file in lottie_files:
    lottie_filename = os.path.basename(api.path.abspath(lottie_file))
    if (not lottie_filename.endswith('.json') or
        lottie_filename in (test_skip_lotties or lottie_samples_to_skip)):
      continue
    trace_file = api.flavor.device_dirs.dm_dir + '/trace-%s' % lottie_filename
    # Sanitize the file name.
    # trace_file = trace_file.replace(' ', '_').replace('(', '').replace(')', '')
    test_data = api.properties.get('trace_test_data', '{}')
    content = api.flavor.read_file_on_device(trace_file)
    if not content and test_data:
      content = test_data
    print 'Got content:'
    print content
    out = api.json.loads(content)
    print 'Trace file contents of %s are:' % lottie_filename
    print out

    # Skip the first "skottie::Animation::seek" constructor call.
    skipped_first_seek = False

    lottie_file_results = {}
    perf_results[lottie_filename] = lottie_file_results

    frame_max = 0
    frame_min = 0
    frame_cummulative = 0
    current_frame_duration = 0
    total_frames = 0
    frame_start = False
    for trace in out:
      print 'Processing trace: %s' % trace
      if 'skottie::Animation::seek' in trace['name']:
        if not skipped_first_seek:
          skipped_first_seek = True
          continue
        if frame_start:
          raise Exception('We got consecutive Animation::seek without a ' +
                          'render. Something is wrong.')
        frame_start = True
        current_frame_duration = trace['dur']
      elif 'skottie::Animation::render' in trace['name']:
        if not frame_start:
          raise Exception('We got an Animation::render without a seek first. ' +
                          'Something is wrong.')

        current_frame_duration += trace['dur']
        frame_start = False
        total_frames += 1
        frame_max = max(frame_max, current_frame_duration)
        frame_min = min(frame_min, current_frame_duration) if frame_min else current_frame_duration
        frame_cummulative += current_frame_duration

    lottie_file_results['frame_max_us'] = frame_max
    lottie_file_results['frame_min_us'] = frame_min
    lottie_file_results['frame_avg_us'] = float("%.2f" % (
        frame_cummulative/total_frames))

  # Construct contents of perf.json
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
  reg = re.compile('Perf-(?P<os>[A-Za-z0-9_]+)-'
                   '(?P<compiler>[A-Za-z0-9_]+)-'
                   '(?P<model>[A-Za-z0-9_]+)-GPU-'
                   '(?P<cpu_or_gpu_value>[A-Za-z0-9_]+)-'
                   '(?P<arch>[A-Za-z0-9_]+)-'
                   '(?P<configuration>[A-Za-z0-9_]+)-'
                   'All(-(?P<extra_config>[A-Za-z0-9_]+)|)')
  keys = ['os', 'compiler', 'model', 'cpu_or_gpu_value', 'arch',
          'configuration', 'extra_config']
  perf_json['key'] = {
      'bench_type': 'tracing',
      'source_type': 'skottie',
  }
  m = reg.match(api.properties['buildername'])
  reg = re.compile('Perf-(?P<os>[A-Za-z0-9_]+)-'
                   '(?P<compiler>[A-Za-z0-9_]+)-'
                   '(?P<model>[A-Za-z0-9_]+)-GPU-'
                   '(?P<cpu_or_gpu_value>[A-Za-z0-9_]+)-'
                   '(?P<arch>[A-Za-z0-9_]+)-'
                   '(?P<configuration>[A-Za-z0-9_]+)-'
                   'All(-(?P<extra_config>[A-Za-z0-9_]+)|)')
  keys = ['os', 'compiler', 'model', 'cpu_or_gpu_value', 'arch',
          'configuration', 'extra_config']
  for k in keys:
    perf_json['key'][k] = m.group(k)
  # Add the tracing results.
  perf_json['results'] = {}
  perf_json['results']['gles'] = perf_results

  # Create the perf.json file.
  api.file.ensure_directory(
      'makedirs perf_dir',
      api.flavor.host_dirs.perf_data_dir)
  api.run(
      api.python.inline,
      'write perf.json',
      program="""import json
with open('%s', 'w') as outfile:
  json.dump(obj=%s, fp=outfile, indent=4)
  """ % (api.flavor.host_dirs.perf_data_dir.join('perf.json'), perf_json))


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
  test_skip_lotties = ['skip_lottie.json']
  trace_output = """
[{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":452,"dur":2.57,"tid":1,"pid":0},{"ph":"X","name":"void SkCanvas::drawPaint(const SkPaint &)","ts":473,"dur":2.67e+03,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":3.15e+03,"dur":2.25,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::render(SkCanvas *, const SkRect *, RenderFlags) const","ts":3.15e+03,"dur":216,"tid":1,"pid":0},{"ph":"X","name":"void SkCanvas::drawPath(const SkPath &, const SkPaint &)","ts":3.35e+03,"dur":15.1,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":3.37e+03,"dur":1.17,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::render(SkCanvas *, const SkRect *, RenderFlags) const","ts":3.37e+03,"dur":140,"tid":1,"pid":0}]
"""
  trace_output_error1 = """
[{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":452,"dur":2.57,"tid":1,"pid":0},{"ph":"X","name":"void SkCanvas::drawPaint(const SkPaint &)","ts":473,"dur":2.67e+03,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":3.15e+03,"dur":2.25,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":3.37e+03,"dur":1.17,"tid":1,"pid":0}]
"""
  trace_output_error2 = """
[{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":452,"dur":2.57,"tid":1,"pid":0},{"ph":"X","name":"void SkCanvas::drawPaint(const SkPaint &)","ts":473,"dur":2.67e+03,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::render(SkCanvas *, const SkRect *, RenderFlags) const","ts":3.15e+03,"dur":216,"tid":1,"pid":0}]
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
                     test_skip_lotties=test_skip_lotties,
                     dm_json_test_data=dm_json_test_data,
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'].join('perf', 'perf.json'))
  )
  yield (
      api.test('skottietracing_error1') +
      api.properties(buildername=('Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-AndroidSkottieTracing'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     task_id='abc123',
                     trace_test_data=trace_output_error1,
                     test_skip_lotties=test_skip_lotties,
                     dm_json_test_data=dm_json_test_data,
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.expect_exception('Exception')
  )
  yield (
      api.test('skottietracing_error2') +
      api.properties(buildername=('Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-AndroidSkottieTracing'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     task_id='abc123',
                     trace_test_data=trace_output_error2,
                     test_skip_lotties=test_skip_lotties,
                     dm_json_test_data=dm_json_test_data,
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.expect_exception('Exception')
  )
  yield (
      api.test('skottietracing_trybot') +
      api.properties(buildername=('Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-AndroidSkottieTracing'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     task_id='abc123',
                     trace_test_data=trace_output,
                     test_skip_lotties=test_skip_lotties,
                     dm_json_test_data=dm_json_test_data,
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
