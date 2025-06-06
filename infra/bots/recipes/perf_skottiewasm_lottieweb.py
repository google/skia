# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which runs Skottie-WASM and Lottie-Web perf.

import calendar
import json
import re

PYTHON_VERSION_COMPATIBILITY = "PY3"

# trim
DEPS = [
  'flavor',
  'checkout',
  'env',
  'infra',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'recipe_engine/time',
  'run',
  'vars',
]

LOTTIE_WEB_EXCLUDE = [
  # See https://bugs.chromium.org/p/skia/issues/detail?id=9187#c4
  'lottiefiles.com - Progress Success.json',
  # Fails with "val2 is not defined".
  'lottiefiles.com - VR.json',
  'vr_animation.json',
  # Times out.
  'lottiefiles.com - Nudge.json',
  'lottiefiles.com - Retweet.json',
  'regress-perspective-blur-01.json',
  'regress-perspective-blur-02.json',
  # Trace file has majority main_frame_aborted terminations in it and < 25
  # occurrences of submitted_frame + missed_frame.
  # Static scenes (nothing animating)
  'mask1.json',
  'mask2.json',
  'stacking.json',
]

SKOTTIE_WASM_EXCLUDE = [
  # Trace file has majority main_frame_aborted terminations in it and < 25
  # occurrences of submitted_frame + missed_frame.
  # Below descriptions are added from fmalita@'s comments in
  # https://skia-review.googlesource.com/c/skia/+/229419

  # Static scenes (nothing animating)
  'mask1.json',
  'mask2.json',
  'stacking.json',
  # Static in Skottie only due to unsupported feature (expressions).
  'dna.json',
  'elephant_trunk_swing.json',
  # Looks all static in both skottie/lottie, not sure why lottie doesn't abort
  # as many frames.
  'hexadots.json',
  # Very short transition, mostly static.
  'screenhole.json',
  # Broken in Skottie due to unidentified missing feature.
  'interleague_golf_logo.json',
  'loading.json',
  'lottiefiles.com - Loading 2.json',
  'streetby_loading.json',
  'streetby_test_loading.json',
  # Times out
  'beetle.json',
  # Too slow? Doesn't provide enough frames for analysis b/325452373
  'Name.json',
]

# These files work in SVG but not in Canvas.
LOTTIE_WEB_CANVAS_EXCLUDE = LOTTIE_WEB_EXCLUDE + [
  'Hello World.json',
  'interactive_menu.json',
  'Name.json',
]


def RunSteps(api):
  api.vars.setup()
  api.flavor.setup(None)
  checkout_root = api.path.start_dir
  buildername = api.properties['buildername']
  node_path = api.path.start_dir.joinpath('node', 'node', 'bin', 'node')
  lottie_files = api.file.listdir(
      'list lottie files', api.flavor.host_dirs.lotties_dir,
      test_data=['lottie1.json', 'lottie2.json', 'lottie3.json', 'LICENSE'])

  if 'SkottieWASM' in buildername:
    source_type = 'skottie'
    renderer = 'skottie-wasm'

    perf_app_dir = checkout_root.joinpath('skia', 'tools', 'skottie-wasm-perf')
    canvaskit_js_path = api.vars.build_dir.joinpath('canvaskit.js')
    canvaskit_wasm_path = api.vars.build_dir.joinpath('canvaskit.wasm')
    skottie_wasm_js_path = perf_app_dir.joinpath('skottie-wasm-perf.js')
    perf_app_cmd = [
        node_path, skottie_wasm_js_path,
        '--canvaskit_js', canvaskit_js_path,
        '--canvaskit_wasm', canvaskit_wasm_path,
    ]
    lottie_files = [x for x in lottie_files
                    if api.path.basename(x) not in SKOTTIE_WASM_EXCLUDE]
  elif 'LottieWeb' in buildername:
    source_type = 'lottie-web'
    renderer = 'lottie-web'
    if 'Canvas' in buildername:
      backend = 'canvas'
      lottie_files = [
          x for x in lottie_files
          if api.path.basename(x) not in LOTTIE_WEB_CANVAS_EXCLUDE]
    else:
      backend = 'svg'
      lottie_files = [x for x in lottie_files
                      if api.path.basename(x) not in LOTTIE_WEB_EXCLUDE]

    perf_app_dir = checkout_root.joinpath('skia', 'tools', 'lottie-web-perf')
    lottie_web_js_path = perf_app_dir.joinpath('lottie-web-perf.js')
    perf_app_cmd = [
        node_path, lottie_web_js_path,
        '--backend', backend,
    ]
  else:
    raise Exception('Could not recognize the buildername %s' % buildername)

  if api.vars.builder_cfg.get('cpu_or_gpu') == 'GPU':
    perf_app_cmd.append('--use_gpu')

  # Install prerequisites.
  env_prefixes = {'PATH': [api.path.start_dir.joinpath('node', 'node', 'bin')]}
  with api.context(cwd=perf_app_dir, env_prefixes=env_prefixes):
    api.step('npm install', cmd=['npm', 'install'])

  perf_results = {}
  output_dir = api.path.mkdtemp('g3_try')
  # Run the perf_app_cmd on each lottie file and parse the trace files.
  for _, lottie_file in enumerate(lottie_files):
    lottie_filename = api.path.basename(lottie_file)
    if not lottie_filename.endswith('.json'):
      continue
    output_file = output_dir.joinpath(lottie_filename)
    with api.context(cwd=perf_app_dir, env={'DISPLAY': ':0'}):
      # This is occasionally flaky due to skbug.com/40040508, adding retries.
      attempts = 3
      # Add output and input arguments to the cmd.
      api.run.with_retry(api.step, 'Run perf cmd line app', attempts,
                         cmd=perf_app_cmd + [
                             '--input', lottie_file,
                             '--output', output_file,
                         ], infra_step=True)

    perf_results[lottie_filename] = {
        'gl': parse_trace(output_file, lottie_filename, api, renderer),
    }

  # Construct contents of the output JSON.
  perf_json = {
      'gitHash': api.properties['revision'],
      'swarming_bot_id': api.vars.swarming_bot_id,
      'swarming_task_id': api.vars.swarming_task_id,
      'key': {
        'bench_type': 'tracing',
        'source_type': source_type,
      },
      'renderer': renderer,
      'results': perf_results,
  }
  if api.vars.is_trybot:
    perf_json['issue'] = api.vars.issue
    perf_json['patchset'] = api.vars.patchset
    perf_json['patch_storage'] = api.vars.patch_storage
  # Add tokens from the builder name to the key.
  reg = re.compile('Perf-(?P<os>[A-Za-z0-9_]+)-'
                   '(?P<compiler>[A-Za-z0-9_]+)-'
                   '(?P<model>[A-Za-z0-9_]+)-'
                   '(?P<cpu_or_gpu>[A-Z]+)-'
                   '(?P<cpu_or_gpu_value>[A-Za-z0-9_]+)-'
                   '(?P<arch>[A-Za-z0-9_]+)-'
                   '(?P<configuration>[A-Za-z0-9_]+)-'
                   'All(-(?P<extra_config>[A-Za-z0-9_]+)|)')
  m = reg.match(api.properties['buildername'])
  keys = ['os', 'compiler', 'model', 'cpu_or_gpu', 'cpu_or_gpu_value', 'arch',
          'configuration', 'extra_config']
  for k in keys:
    perf_json['key'][k] = m.group(k)

  # Create the output JSON file in perf_data_dir for the Upload task to upload.
  api.file.ensure_directory(
      'makedirs perf_dir',
      api.flavor.host_dirs.perf_data_dir)
  now = api.time.utcnow()
  ts = int(calendar.timegm(now.utctimetuple()))
  json_path = api.flavor.host_dirs.perf_data_dir.joinpath(
      'perf_%s_%d.json' % (api.properties['revision'], ts))
  json_contents = json.dumps(
      perf_json, indent=4, sort_keys=True, separators=(',', ': '))
  api.file.write_text('write output JSON', json_path, json_contents)


def parse_trace(trace_json, lottie_filename, api, renderer):
  """parse_trace parses the specified trace JSON.

  Parses the trace JSON and calculates the time of a single frame.
  A dictionary is returned that has the following structure:
  {
    'frame_max_us': 100,
    'frame_min_us': 90,
    'frame_avg_us': 95,
  }
  """
  script = api.infra.resource('parse_lottieweb_trace.py')
  step_result = api.run(
      api.step,
      'parse %s trace' % lottie_filename,
      cmd=['python3', script, trace_json, api.json.output(), renderer])

  # Sanitize float outputs to 2 precision points.
  output = dict(step_result.json.output)
  output['frame_max_us'] = float("%.2f" % output['frame_max_us'])
  output['frame_min_us'] = float("%.2f" % output['frame_min_us'])
  output['frame_avg_us'] = float("%.2f" % output['frame_avg_us'])
  return output


def GenTests(api):
  trace_output = """
[{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":452,"dur":2.57,"tid":1,"pid":0},{"ph":"X","name":"void SkCanvas::drawPaint(const SkPaint &)","ts":473,"dur":2.67e+03,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":3.15e+03,"dur":2.25,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::render(SkCanvas *, const SkRect *, RenderFlags) const","ts":3.15e+03,"dur":216,"tid":1,"pid":0},{"ph":"X","name":"void SkCanvas::drawPath(const SkPath &, const SkPaint &)","ts":3.35e+03,"dur":15.1,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::seek(SkScalar)","ts":3.37e+03,"dur":1.17,"tid":1,"pid":0},{"ph":"X","name":"void skottie::Animation::render(SkCanvas *, const SkRect *, RenderFlags) const","ts":3.37e+03,"dur":140,"tid":1,"pid":0}]
"""
  parse_trace_json = {
      'frame_avg_us': 179.71,
      'frame_min_us': 141.17,
      'frame_max_us': 218.25
  }


  skottie_cpu_buildername = ('Perf-Debian10-EMCC-GCE-CPU-AVX2-wasm-Release-All-'
                             'SkottieWASM')
  yield (
      api.test('skottie_wasm_perf') +
      api.properties(buildername=skottie_cpu_buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     trace_test_data=trace_output,
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('parse lottie1.json trace',
                    api.json.output(parse_trace_json)) +
      api.step_data('parse lottie2.json trace',
                    api.json.output(parse_trace_json)) +
      api.step_data('parse lottie3.json trace',
                    api.json.output(parse_trace_json))
  )
  yield (
      api.test('skottie_wasm_perf_trybot') +
      api.properties(buildername=skottie_cpu_buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     trace_test_data=trace_output,
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     patch_ref='89/456789/12',
                     patch_repo='https://skia.googlesource.com/skia.git',
                     patch_storage='gerrit',
                     patch_set=7,
                     patch_issue=1234,
                     gerrit_project='skia',
                     gerrit_url='https://skia-review.googlesource.com/') +
      api.step_data('parse lottie1.json trace',
                    api.json.output(parse_trace_json)) +
      api.step_data('parse lottie2.json trace',
                    api.json.output(parse_trace_json)) +
      api.step_data('parse lottie3.json trace',
                    api.json.output(parse_trace_json))
  )

  skottie_gpu_buildername = ('Perf-Debian10-EMCC-NUC7i5BNK-GPU-IntelIris640-'
                             'wasm-Release-All-SkottieWASM')
  yield (
      api.test('skottie_wasm_perf_gpu') +
      api.properties(buildername=skottie_gpu_buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     trace_test_data=trace_output,
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('parse lottie1.json trace',
                    api.json.output(parse_trace_json)) +
      api.step_data('parse lottie2.json trace',
                    api.json.output(parse_trace_json)) +
      api.step_data('parse lottie3.json trace',
                    api.json.output(parse_trace_json))
  )

  lottieweb_cpu_buildername = ('Perf-Debian10-none-GCE-CPU-AVX2-x86_64-Release-'
                               'All-LottieWeb')
  yield (
      api.test('lottie_web_perf') +
      api.properties(buildername=lottieweb_cpu_buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     trace_test_data=trace_output,
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('parse lottie1.json trace',
                    api.json.output(parse_trace_json)) +
      api.step_data('parse lottie2.json trace',
                    api.json.output(parse_trace_json)) +
      api.step_data('parse lottie3.json trace',
                    api.json.output(parse_trace_json))
  )
  yield (
      api.test('lottie_web_perf_trybot') +
      api.properties(buildername=lottieweb_cpu_buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     trace_test_data=trace_output,
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     patch_ref='89/456789/12',
                     patch_repo='https://skia.googlesource.com/skia.git',
                     patch_storage='gerrit',
                     patch_set=7,
                     patch_issue=1234,
                     gerrit_project='skia',
                     gerrit_url='https://skia-review.googlesource.com/') +
      api.step_data('parse lottie1.json trace',
                    api.json.output(parse_trace_json)) +
      api.step_data('parse lottie2.json trace',
                    api.json.output(parse_trace_json)) +
      api.step_data('parse lottie3.json trace',
                    api.json.output(parse_trace_json))
  )

  lottieweb_canvas_cpu_buildername = (
      'Perf-Debian10-none-GCE-CPU-AVX2-x86_64-Release-All-LottieWeb_Canvas')
  yield (
      api.test('lottie_web_canvas_perf') +
      api.properties(buildername=lottieweb_canvas_cpu_buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     trace_test_data=trace_output,
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('parse lottie1.json trace',
                    api.json.output(parse_trace_json)) +
      api.step_data('parse lottie2.json trace',
                    api.json.output(parse_trace_json)) +
      api.step_data('parse lottie3.json trace',
                    api.json.output(parse_trace_json))
  )
  yield (
      api.test('lottie_web_canvas_perf_trybot') +
      api.properties(buildername=lottieweb_canvas_cpu_buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     trace_test_data=trace_output,
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     patch_ref='89/456789/12',
                     patch_repo='https://skia.googlesource.com/skia.git',
                     patch_storage='gerrit',
                     patch_set=7,
                     patch_issue=1234,
                     gerrit_project='skia',
                     gerrit_url='https://skia-review.googlesource.com/') +
      api.step_data('parse lottie1.json trace',
                    api.json.output(parse_trace_json)) +
      api.step_data('parse lottie2.json trace',
                    api.json.output(parse_trace_json)) +
      api.step_data('parse lottie3.json trace',
                    api.json.output(parse_trace_json))
  )

  unrecognized_buildername = ('Perf-Debian10-none-GCE-CPU-AVX2-x86_64-Release-'
                              'All-Unrecognized')
  yield (
      api.test('unrecognized_builder') +
      api.properties(buildername=unrecognized_buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.expect_exception('Exception')
  )
