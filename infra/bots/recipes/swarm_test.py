# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming test.


DEPS = [
  'build/file',
  'core',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'flavor',
  'run',
  'vars',
]


TEST_BUILDERS = {
  'client.skia': {
    'skiabot-linux-swarm-000': [
      'Test-Android-GCC-GalaxyS3-GPU-Mali400-Arm7-Debug',
      'Test-Android-GCC-Nexus7-GPU-Tegra3-Arm7-Debug',
      'Test-iOS-Clang-iPad4-GPU-SGX554-Arm7-Debug',
      'Test-Ubuntu-Clang-GCE-CPU-AVX2-x86_64-Coverage-Trybot',
      'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug',
      'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-MSAN',
      'Test-Ubuntu-GCC-ShuttleA-GPU-GTX550Ti-x86_64-Release-Valgrind',
      'Test-Win8-MSVC-ShuttleB-CPU-AVX2-x86_64-Release-Trybot',
    ],
  },
}


def key_params(api):
  """Build a unique key from the builder name (as a list).

  E.g.  arch x86 gpu GeForce320M mode MacMini4.1 os Mac10.6
  """
  # Don't bother to include role, which is always Test.
  # TryBots are uploaded elsewhere so they can use the same key.
  blacklist = ['role', 'is_trybot']

  flat = []
  for k in sorted(api.vars.builder_cfg.keys()):
    if k not in blacklist:
      flat.append(k)
      flat.append(api.vars.builder_cfg[k])
  return flat


def test_steps(api):
  """Run the DM test."""
  use_hash_file = False
  if api.vars.upload_dm_results:
    # This must run before we write anything into
    # api.flavor.device_dirs.dm_dir or we may end up deleting our
    # output on machines where they're the same.
    api.flavor.create_clean_host_dir(api.vars.dm_dir)
    host_dm_dir = str(api.vars.dm_dir)
    device_dm_dir = str(api.flavor.device_dirs.dm_dir)
    if host_dm_dir != device_dm_dir:
      api.flavor.create_clean_device_dir(device_dm_dir)

    # Obtain the list of already-generated hashes.
    hash_filename = 'uninteresting_hashes.txt'

    # Ensure that the tmp_dir exists.
    api.run.run_once(api.file.makedirs,
                           'tmp_dir',
                           api.vars.tmp_dir,
                           infra_step=True)

    host_hashes_file = api.vars.tmp_dir.join(hash_filename)
    hashes_file = api.flavor.device_path_join(
        api.flavor.device_dirs.tmp_dir, hash_filename)
    api.run(
        api.python.inline,
        'get uninteresting hashes',
        program="""
        import contextlib
        import math
        import socket
        import sys
        import time
        import urllib2

        HASHES_URL = 'https://gold.skia.org/_/hashes'
        RETRIES = 5
        TIMEOUT = 60
        WAIT_BASE = 15

        socket.setdefaulttimeout(TIMEOUT)
        for retry in range(RETRIES):
          try:
            with contextlib.closing(
                urllib2.urlopen(HASHES_URL, timeout=TIMEOUT)) as w:
              hashes = w.read()
              with open(sys.argv[1], 'w') as f:
                f.write(hashes)
                break
          except Exception as e:
            print 'Failed to get uninteresting hashes from %s:' % HASHES_URL
            print e
            if retry == RETRIES:
              raise
            waittime = WAIT_BASE * math.pow(2, retry)
            print 'Retry in %d seconds.' % waittime
            time.sleep(waittime)
        """,
        args=[host_hashes_file],
        cwd=api.vars.skia_dir,
        abort_on_failure=False,
        fail_build_on_failure=False,
        infra_step=True)

    if api.path.exists(host_hashes_file):
      api.flavor.copy_file_to_device(host_hashes_file, hashes_file)
      use_hash_file = True

  # Run DM.
  properties = [
    'gitHash',      api.vars.got_revision,
    'master',       api.vars.master_name,
    'builder',      api.vars.builder_name,
    'build_number', api.vars.build_number,
  ]
  if api.vars.is_trybot:
    properties.extend([
      'issue',    api.vars.issue,
      'patchset', api.vars.patchset,
    ])

  args = [
    'dm',
    '--undefok',   # This helps branches that may not know new flags.
    '--resourcePath', api.flavor.device_dirs.resource_dir,
    '--skps', api.flavor.device_dirs.skp_dir,
    '--images', api.flavor.device_path_join(
        api.flavor.device_dirs.images_dir, 'dm'),
    '--colorImages', api.flavor.device_path_join(
        api.flavor.device_dirs.images_dir, 'colorspace'),
    '--nameByHash',
    '--properties'
  ] + properties

  args.append('--key')
  args.extend(key_params(api))
  if use_hash_file:
    args.extend(['--uninterestingHashesFile', hashes_file])
  if api.vars.upload_dm_results:
    args.extend(['--writePath', api.flavor.device_dirs.dm_dir])

  skip_flag = None
  if api.vars.builder_cfg.get('cpu_or_gpu') == 'CPU':
    skip_flag = '--nogpu'
  elif api.vars.builder_cfg.get('cpu_or_gpu') == 'GPU':
    skip_flag = '--nocpu'
  if skip_flag:
    args.append(skip_flag)
  args.extend(api.vars.builder_spec['dm_flags'])

  api.run(api.flavor.step, 'dm', cmd=args,
          abort_on_failure=False,
          env=api.vars.default_env)

  if api.vars.upload_dm_results:
    # Copy images and JSON to host machine if needed.
    api.flavor.copy_directory_contents_to_host(
        api.flavor.device_dirs.dm_dir, api.vars.dm_dir)

  # See skia:2789.
  if ('Valgrind' in api.vars.builder_name and
      api.vars.builder_cfg.get('cpu_or_gpu') == 'GPU'):
    abandonGpuContext = list(args)
    abandonGpuContext.append('--abandonGpuContext')
    api.run(api.flavor.step, 'dm --abandonGpuContext',
                  cmd=abandonGpuContext, abort_on_failure=False)
    preAbandonGpuContext = list(args)
    preAbandonGpuContext.append('--preAbandonGpuContext')
    api.run(api.flavor.step, 'dm --preAbandonGpuContext',
                  cmd=preAbandonGpuContext, abort_on_failure=False,
                  env=api.vars.default_env)


def RunSteps(api):
  api.core.setup()
  api.flavor.install()
  test_steps(api)
  api.flavor.cleanup_steps()
  api.run.check_failure()


def GenTests(api):
  def AndroidTestData(builder, adb=None):
    test_data = (
        api.step_data(
            'get EXTERNAL_STORAGE dir',
            stdout=api.raw_io.output('/storage/emulated/legacy')) +
        api.step_data(
            'read SKP_VERSION',
            stdout=api.raw_io.output('42')) +
        api.step_data(
            'read SK_IMAGE_VERSION',
            stdout=api.raw_io.output('42')) +
       api.step_data(
            'exists skia_dm',
            stdout=api.raw_io.output(''))
    )
    if 'GalaxyS3' not in builder:
      test_data += api.step_data(
          'adb root',
          stdout=api.raw_io.output('restarting adbd as root'))
    if adb:
      test_data += api.step_data(
          'which adb',
          stdout=api.raw_io.output(adb))
    else:
      test_data += api.step_data(
        'which adb',
        retcode=1)

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

  builder = 'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug'
  yield (
    api.test('failed_dm') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
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
    ) +
    api.step_data('dm', retcode=1)
  )

  builder = 'Test-Android-GCC-Nexus7-GPU-Tegra3-Arm7-Debug'
  yield (
    api.test('failed_get_hashes') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
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
    ) +
    AndroidTestData(builder) +
    api.step_data('read SKP_VERSION',
                  stdout=api.raw_io.output('42')) +
    api.step_data('read SK_IMAGE_VERSION',
                  stdout=api.raw_io.output('42')) +
    api.step_data('get uninteresting hashes', retcode=1)
  )

  yield (
    api.test('download_and_push_skps') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
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
    ) +
    AndroidTestData(builder) +
    api.step_data('read SKP_VERSION',
                  stdout=api.raw_io.output('2')) +
    api.step_data('read SK_IMAGE_VERSION',
                  stdout=api.raw_io.output('42')) +
    api.step_data(
        'exists skps',
        stdout=api.raw_io.output(''))
  )

  yield (
    api.test('missing_SKP_VERSION_device') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
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
    ) +
    AndroidTestData(builder) +
    api.step_data('read SKP_VERSION',
                  retcode=1) +
    api.step_data('read SK_IMAGE_VERSION',
                  stdout=api.raw_io.output('42')) +
    api.step_data(
        'exists skps',
        stdout=api.raw_io.output(''))
  )

  yield (
    api.test('download_and_push_skimage') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
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
    ) +
    AndroidTestData(builder) +
    api.step_data('read SKP_VERSION',
                  stdout=api.raw_io.output('42')) +
    api.step_data('read SK_IMAGE_VERSION',
                  stdout=api.raw_io.output('2')) +
    api.step_data(
        'exists skia_images',
        stdout=api.raw_io.output(''))
  )

  yield (
    api.test('missing_SK_IMAGE_VERSION_device') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
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
    ) +
    AndroidTestData(builder) +
    api.step_data('read SKP_VERSION',
                  stdout=api.raw_io.output('42')) +
    api.step_data('read SK_IMAGE_VERSION',
                  retcode=1) +
    api.step_data(
        'exists skia_images',
        stdout=api.raw_io.output(''))
  )

  yield (
    api.test('adb_in_path') +
    api.properties(buildername=builder,
                   mastername='client.skia',
                   slavename='skiabot-linux-swarm-000',
                   buildnumber=6,
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
    ) +
    AndroidTestData(builder, adb='/usr/bin/adb') +
    api.step_data('read SKP_VERSION',
                  stdout=api.raw_io.output('42')) +
    api.step_data('read SK_IMAGE_VERSION',
                  stdout=api.raw_io.output('42'))
  )

  builder = 'Test-Win8-MSVC-ShuttleB-CPU-AVX2-x86_64-Release-Trybot'
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
