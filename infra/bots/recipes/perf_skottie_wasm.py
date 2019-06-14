# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which runs Skottie-WASM perf.

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
  'run',
  'vars',
]


def RunSteps(api):
  api.vars.setup()
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)
  api.flavor.setup()
  checkout_root = api.checkout.default_checkout_root
  skottie_wasm_perf_dir = checkout_root.join('skia', 'tools',
                                             'skottie-wasm-perf')
  out_dir = api.vars.swarming_out_dir

  print "------------------------"
  api.checkout.bot_update(checkout_root=checkout_root)
  print checkout_root
  print skottie_wasm_perf_dir
  print "------------------------"

  # Install prerequisites.
  env = {}
  with api.context(cwd=skottie_wasm_perf_dir, env=env):
    api.step('npm install', cmd=['npm', 'install'])

  skottie_wasm_js_path = skottie_wasm_perf_dir.join('skottie-wasm.js')
  lottie_files = api.file.listdir(
      'list lottie files', api.flavor.host_dirs.lotties_dir,
      test_data=['lottie1.json', 'lottie2.json', 'lottie3.json', 'LICENSE'])
  # Run skottie_wask.js on each lottie file and parse the trace files.
  for idx, lottie_file in enumerate(lottie_files):
    lottie_filename = api.path.basename(lottie_file)
    if not lottie_filename.endswith('.json'):
      continue
    with api.context(cwd=skottie_wasm_perf_dir, env=env):
      api.step('Run skottie-wasm.js', cmd=[
          'node', skottie_wasm_js_path, '--input', lottie_file])


def GenTests(api):
  cpu_buildername = ('Perf-Debian9-Clang-GCE-CPU-AVX2-x86_64-Release-All-'
                     'SkottieWASM')
  yield (
      api.test(cpu_buildername) +
      api.properties(buildername=cpu_buildername,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('skottie_wasm_perf_trybot') +
      api.properties(buildername=cpu_buildername,
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
