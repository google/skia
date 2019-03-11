# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which runs the PathKit tests using docker

DEPS = [
  'checkout',
  'env',
  'infra',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
  'run',
  'vars',
]


def upload_perf_results():
  # Any reason now to always upload perf results?
  return True


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

  # TODO add all necessary flags.

  # Run DM once for all lottie files.

  # Grab the trace file and convert it to a perf file..

  api.run(api.flavor.step, target, cmd=args,
          abort_on_failure=False)

  # Copy results to swarming out dir.
  if upload_perf_results(b):
    api.file.ensure_directory(
        'makedirs perf_dir',
        api.flavor.host_dirs.perf_data_dir)
    api.flavor.copy_directory_contents_to_host(
        api.flavor.device_dirs.perf_data_dir,
        api.flavor.host_dirs.perf_data_dir)

  # Upload step does the actual upload?


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
  yield (
      api.test('Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-AndroidSkottieTracing') +
      api.properties(buildername=('Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-AndroidSkottieTracing'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('skottietracing_trybot') +
      api.properties(buildername=('Perf-Android-Clang-AndroidOne-GPU-Mali400MP2-arm-Release-All-AndroidSkottieTracing'),
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
