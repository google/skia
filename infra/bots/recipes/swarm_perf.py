# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming perf.


DEPS = [
  'build/file',
  'core',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/raw_io',
  'run',
  'flavor',
  'vars',
]


TEST_BUILDERS = {
  'client.skia': {
    'skiabot-linux-swarm-000': [
      'Perf-Win-MSVC-GCE-CPU-AVX2-x86_64-Release',
      'Perf-Win-MSVC-GCE-CPU-AVX2-x86_64-Debug',
      'Perf-Win8-MSVC-ShuttleB-GPU-HD4600-x86_64-Release-Trybot',
      'Perf-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-Valgrind',
      'Perf-Android-GCC-Nexus7-GPU-Tegra3-Arm7-Release',
      'Perf-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-VisualBench',
    ],
  },
}


def perf_steps(api):
  """Run Skia benchmarks."""
  if api.vars.upload_perf_results:
    api.flavor.create_clean_device_dir(
        api.flavor.device_dirs.perf_data_dir)

  # Run nanobench.
  properties = [
    '--properties',
    'gitHash',      api.vars.got_revision,
    'build_number', api.vars.build_number,
  ]
  if api.vars.is_trybot:
    properties.extend([
      'issue',    api.vars.issue,
      'patchset', api.vars.patchset,
    ])

  target = 'nanobench'
  if 'VisualBench' in api.vars.builder_name:
    target = 'visualbench'
  args = [
      target,
      '--undefok',   # This helps branches that may not know new flags.
      '-i',       api.flavor.device_dirs.resource_dir,
      '--skps',   api.flavor.device_dirs.skp_dir,
      '--images', api.flavor.device_path_join(
          api.flavor.device_dirs.images_dir, 'nanobench'),
  ]

  skip_flag = None
  if api.vars.builder_cfg.get('cpu_or_gpu') == 'CPU':
    skip_flag = '--nogpu'
  elif api.vars.builder_cfg.get('cpu_or_gpu') == 'GPU':
    skip_flag = '--nocpu'
  if skip_flag:
    args.append(skip_flag)
  args.extend(api.vars.builder_spec['nanobench_flags'])

  if api.vars.upload_perf_results:
    json_path = api.flavor.device_path_join(
        api.flavor.device_dirs.perf_data_dir,
        'nanobench_%s.json' % api.vars.got_revision)
    args.extend(['--outResultsFile', json_path])
    args.extend(properties)

    keys_blacklist = ['configuration', 'role', 'is_trybot']
    args.append('--key')
    for k in sorted(api.vars.builder_cfg.keys()):
      if not k in keys_blacklist:
        args.extend([k, api.vars.builder_cfg[k]])

  api.run(api.flavor.step, target, cmd=args,
          abort_on_failure=False,
          env=api.vars.default_env)

  # See skia:2789.
  if ('Valgrind' in api.vars.builder_name and
      api.vars.builder_cfg.get('cpu_or_gpu') == 'GPU'):
    abandonGpuContext = list(args)
    abandonGpuContext.extend(['--abandonGpuContext', '--nocpu'])
    api.run(api.flavor.step,
            '%s --abandonGpuContext' % target,
            cmd=abandonGpuContext, abort_on_failure=False,
            env=api.vars.default_env)

  # Copy results to swarming out dir.
  if api.vars.upload_perf_results:
    api.file.makedirs('perf_dir', api.vars.perf_data_dir)
    api.flavor.copy_directory_contents_to_host(
        api.flavor.device_dirs.perf_data_dir,
        api.vars.perf_data_dir)


def RunSteps(api):
  api.core.setup()
  api.flavor.install()
  perf_steps(api)
  api.flavor.cleanup_steps()
  api.run.check_failure()


def GenTests(api):
  def AndroidTestData(builder):
    test_data = (
        api.step_data(
            'get EXTERNAL_STORAGE dir',
            stdout=api.raw_io.output('/storage/emulated/legacy')) +
        api.step_data(
            'adb root',
            stdout=api.raw_io.output('restarting adbd as root')) +
        api.step_data(
            'read SKP_VERSION',
            stdout=api.raw_io.output('42')) +
        api.step_data(
            'read SK_IMAGE_VERSION',
            stdout=api.raw_io.output('42')) +
        api.step_data(
            'exists skia_perf',
            stdout=api.raw_io.output('')) +
        api.step_data(
            'which adb',
            retcode=1)
    )
    return test_data

  for mastername, slaves in TEST_BUILDERS.iteritems():
    for slavename, builders_by_slave in slaves.iteritems():
      for builder in builders_by_slave:
        test = (
          api.test(builder) +
          api.properties(buildername=builder,
                         mastername=mastername,
                         slavename=slavename,
                         buildnumber=5,
                         revision='abc123',
                         path_config='kitchen',
                         swarm_out_dir='[SWARM_OUT_DIR]') +
          api.path.exists(
              api.path['slave_build'].join('skia'),
              api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                           'skimage', 'VERSION'),
              api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                           'skp', 'VERSION'),
              api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
          )
        )
        if ('Android' in builder and
            ('Test' in builder or 'Perf' in builder) and
            not 'Appurify' in builder):
          test += AndroidTestData(builder)
        if 'Trybot' in builder:
          test += api.properties(issue=500,
                                 patchset=1,
                                 rietveld='https://codereview.chromium.org')
        if 'Win' in builder:
          test += api.platform('win', 64)

        yield test

  builder = 'Perf-Win8-MSVC-ShuttleB-GPU-HD4600-x86_64-Release-Trybot'
  yield (
    api.test('big_issue_number') +
    api.properties(buildername=builder,
                   mastername='client.skia.compile',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=5,
                   revision='abc123',
                   path_config='kitchen',
                   swarm_out_dir='[SWARM_OUT_DIR]',
                   rietveld='https://codereview.chromium.org',
                   patchset=1,
                   issue=2147533002L) +
    api.path.exists(
        api.path['slave_build'].join('skia'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
        api.path['slave_build'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
        api.path['slave_build'].join('tmp', 'uninteresting_hashes.txt')
    ) +
    api.platform('win', 64)
  )
