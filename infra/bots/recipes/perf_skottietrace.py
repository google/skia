# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which runs DM with trace flag on Lottie files.
# Design doc: go/skottie-tracing


import os
import re
import string


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

SEEK_TRACE_NAME = 'skottie::Animation::seek'
RENDER_TRACE_NAME = 'skottie::Animation::render'
EXPECTED_DM_FRAMES = 25


def perf_steps(api):
  """Run DM on lottie files with tracing turned on"""
  api.flavor.create_clean_device_dir(
        api.flavor.device_dirs.dm_dir)

  lottie_files = api.file.listdir(
      'list lottie files', api.flavor.host_dirs.lotties_dir,
      test_data=['lottie1.json', 'lottie(test)\'!2.json', 'lottie 3!.json',
                 'skip_lottie.json', 'LICENSE'])
  perf_results = {}
  # Run DM on each lottie file and parse the trace files.
  push_dm = True
  for idx, lottie_file in enumerate(lottie_files):
    lottie_filename = os.path.basename(api.path.abspath(lottie_file))
    if not lottie_filename.endswith('.json'):
      continue
    trace_output = api.flavor.device_dirs.dm_dir + '/%s.json' % (idx + 1)
    trace_match = '^%s$' % lottie_filename
    if ' ' not in trace_match:
      # Some characters confuse DM, escape them. Do not need to do this when
      # there is a space because subprocess.list2cmdline  automatically adds
      # quotes.
      for sp_char in string.punctuation:
        if sp_char == '\\':
          # No need to escape the escape char.
          continue
        trace_match = trace_match.replace(sp_char, '\%s' % sp_char)
    # See go/skottie-tracing for how these flags were selected.
    dm_args = [
      'dm',
      '--resourcePath', api.flavor.device_dirs.resource_dir,
      '--lotties', api.flavor.device_dirs.lotties_dir,
      '--nocpu',
      '--config', 'gles',
      '--src', 'lottie',
      '--nonativeFonts',
      '--verbose',
      '--traceMatch', 'skottie',  # recipe can OOM without this.
      '--trace', trace_output,
      '--match', trace_match,
    ]
    api.run(api.flavor.step, 'dm', cmd=dm_args, abort_on_failure=False,
            skip_binary_push=not push_dm)
    # We already pushed the binary once. No need to waste time by pushing
    # the same binary repeatedly.
    push_dm = False

    test_data = api.properties.get('trace_test_data', '{}')
    trace_file_content = api.flavor.read_file_on_device(trace_output)
    if not trace_file_content and test_data:
      trace_file_content = test_data
    trace_json = api.json.loads(trace_file_content)

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
    for trace in trace_json:
      print 'Parsing trace: %s' % trace
      if SEEK_TRACE_NAME in trace['name']:
        if not skipped_first_seek:
          skipped_first_seek = True
          continue
        if frame_start:
          raise Exception('We got consecutive Animation::seek without a ' +
                          'render. Something is wrong.')
        frame_start = True
        current_frame_duration = trace['dur']
      elif RENDER_TRACE_NAME in trace['name']:
        if not frame_start:
          raise Exception('We got an Animation::render without a seek first. ' +
                          'Something is wrong.')

        current_frame_duration += trace['dur']
        frame_start = False
        total_frames += 1
        frame_max = max(frame_max, current_frame_duration)
        frame_min = (min(frame_min, current_frame_duration)
                     if frame_min else current_frame_duration)
        frame_cummulative += current_frame_duration

    expected_dm_frames = EXPECTED_DM_FRAMES
    if api.properties.get('test_expected_dm_frames', 0):
      expected_dm_frames = api.properties['test_expected_dm_frames']
    if total_frames != expected_dm_frames:
      raise Exception('%s had %d frames instead of %d', lottie_filename,
                      total_frames, expected_dm_frames)
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


def GenTests(api):
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
  test_expected_dm_frames = 2
  buildername = ('Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-'
                  'All-Android_SkottieTracing')
  yield (
      api.test(buildername) +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     task_id='abc123',
                     trace_test_data=trace_output,
                     dm_json_test_data=dm_json_test_data,
                     test_expected_dm_frames=test_expected_dm_frames,
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'].join('perf', 'perf.json'))
  )
  yield (
      api.test('skottietracing_num_frames_error') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     task_id='abc123',
                     trace_test_data=trace_output,
                     dm_json_test_data=dm_json_test_data,
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.expect_exception('Exception')
  )
  yield (
      api.test('skottietracing_consecutive_seeks_error') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     task_id='abc123',
                     trace_test_data=trace_output_error1,
                     dm_json_test_data=dm_json_test_data,
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.expect_exception('Exception')
  )
  yield (
      api.test('skottietracing_consecutive_renders_error') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     task_id='abc123',
                     trace_test_data=trace_output_error2,
                     dm_json_test_data=dm_json_test_data,
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.expect_exception('Exception')
  )
  yield (
      api.test('skottietracing_trybot') +
      api.properties(buildername=buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     task_id='abc123',
                     trace_test_data=trace_output,
                     dm_json_test_data=dm_json_test_data,
                     test_expected_dm_frames=test_expected_dm_frames,
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
