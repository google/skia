# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which runs Skottie-WASM and Lottie-Web perf.

import calendar
import re


# trim
DEPS = [
  'flavor',
  'checkout',
  'env',
  'infra',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
  'recipe_engine/tempfile',
  'recipe_engine/time',
  'run',
  'vars',
]

LOTTIE_WEB_BLACKLIST = [
  # See https://bugs.chromium.org/p/skia/issues/detail?id=9187#c4
  'lottiefiles.com - Progress Success.json',
  # Fails with "val2 is not defined".
  'lottiefiles.com - VR.json',
  'vr_animation.json',
  # Times out.
  'obama_caricature.json',
  'lottiefiles.com - Nudge.json',
  'lottiefiles.com - Retweet.json',
]


def RunSteps(api):
  api.vars.setup()
  api.flavor.setup()
  checkout_root = api.checkout.default_checkout_root
  api.checkout.bot_update(checkout_root=checkout_root)
  buildername = api.properties['buildername']
  node_path = api.path['start_dir'].join('node', 'node', 'bin', 'node')
  lottie_files = api.file.listdir(
      'list lottie files', api.flavor.host_dirs.lotties_dir,
      test_data=['lottie1.json', 'lottie2.json', 'lottie3.json', 'LICENSE'])

  if 'SkottieWASM' in buildername:
    source_type = 'skottie'
    renderer = 'skottie-wasm'

    perf_app_dir = checkout_root.join('skia', 'tools', 'skottie-wasm-perf')
    canvaskit_js_path = api.vars.build_dir.join('canvaskit.js')
    canvaskit_wasm_path = api.vars.build_dir.join('canvaskit.wasm')
    skottie_wasm_js_path = perf_app_dir.join('skottie-wasm-perf.js')
    perf_app_cmd = [
        node_path, skottie_wasm_js_path,
        '--canvaskit_js', canvaskit_js_path,
        '--canvaskit_wasm', canvaskit_wasm_path,
    ]
  elif 'LottieWeb' in buildername:
    source_type = 'lottie-web'
    renderer = 'lottie-web'

    perf_app_dir = checkout_root.join('skia', 'tools', 'lottie-web-perf')
    lottie_web_js_path = perf_app_dir.join('lottie-web-perf.js')
    perf_app_cmd = [node_path, lottie_web_js_path]
    lottie_files = [x for x in lottie_files
                    if api.path.basename(x) not in LOTTIE_WEB_BLACKLIST]
  else:
    raise Exception('Could not recognize the buildername %s' % buildername)

  # Install prerequisites.
  env_prefixes = {'PATH': [api.path['start_dir'].join('node', 'node', 'bin')]}
  with api.context(cwd=perf_app_dir, env_prefixes=env_prefixes):
    api.step('npm install', cmd=['npm', 'install'])

  perf_results = {}
  with api.tempfile.temp_dir('g3_try') as output_dir:
    # Run the perf_app_cmd on each lottie file and parse the trace files.
    for _, lottie_file in enumerate(lottie_files):
      lottie_filename = api.path.basename(lottie_file)
      if not lottie_filename.endswith('.json'):
        continue
      output_file = output_dir.join(lottie_filename)
      with api.context(cwd=perf_app_dir):
        # This is occasionally flaky due to skbug.com/9207, adding retries.
        attempts = 3
        # Add output and input arguments to the cmd.
        api.run.with_retry(api.step, 'Run perf cmd line app', attempts,
                           cmd=perf_app_cmd + [
                               '--input', lottie_file,
                               '--output', output_file,
                           ], infra_step=True)
      output_json = api.file.read_json(
          'Read perf json', output_file,
          test_data={'frame_avg_us': 185.79982221126556,
                     'frame_max_us': 860.0000292062759,
                     'frame_min_us': 84.9999487400055}
      )
      perf_results[lottie_filename] = {
          'gl': {
              'frame_avg_us': float('%.2f' % output_json['frame_avg_us']),
              'frame_max_us': float('%.2f' % output_json['frame_max_us']),
              'frame_min_us': float('%.2f' % output_json['frame_min_us']),
          }
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
  json_path = api.flavor.host_dirs.perf_data_dir.join(
      'perf_%s_%d.json' % (api.properties['revision'], ts))
  api.run(
      api.python.inline,
      'write output JSON',
      program="""import json
with open('%s', 'w') as outfile:
  json.dump(obj=%s, fp=outfile, indent=4)
  """ % (json_path, perf_json))


def GenTests(api):
  skottie_cpu_buildername = ('Perf-Debian9-EMCC-GCE-CPU-AVX2-wasm-Release-All-'
                             'SkottieWASM')
  yield (
      api.test('skottie_wasm_perf') +
      api.properties(buildername=skottie_cpu_buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
  yield (
      api.test('skottie_wasm_perf_trybot') +
      api.properties(buildername=skottie_cpu_buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
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

  lottieweb_cpu_buildername = ('Perf-Debian9-none-GCE-CPU-AVX2-x86_64-Release-'
                               'All-LottieWeb')
  yield (
      api.test('lottie_web_perf') +
      api.properties(buildername=lottieweb_cpu_buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
  yield (
      api.test('lottie_web_perf_trybot') +
      api.properties(buildername=lottieweb_cpu_buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
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

  unrecognized_buildername = ('Perf-Debian9-none-GCE-CPU-AVX2-x86_64-Release-'
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
