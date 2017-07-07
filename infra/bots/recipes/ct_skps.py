# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import math


DEPS = [
  'core',
  'ct',
  'depot_tools/gsutil',
  'flavor',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'recipe_engine/time',
  'run',
  'skia_swarming',
  'vars',
]


SKPS_VERSION_FILE = 'skps_version'
CT_SKPS_ISOLATE = 'ct_skps.isolate'

# Do not batch archive more slaves than this value. This is used to prevent
# no output timeouts in the 'isolate tests' step.
MAX_SLAVES_TO_BATCHARCHIVE = 100

TOOL_TO_DEFAULT_SKPS_PER_SLAVE = {
    'dm': 10000,
    'nanobench': 1000,
    'get_images_from_skps': 10000,
}

# The SKP repository to use.
DEFAULT_SKPS_CHROMIUM_BUILD = 'c37e844a6f8708-eee762104c75bd'


def RunSteps(api):
  # Figure out which repository to use.
  buildername = api.properties['buildername']
  if '1k' in buildername:
    ct_page_type = '10k'
    num_pages = 1000
  elif '10k' in buildername:
    ct_page_type = '10k'
    num_pages = 10000
  elif '100k' in buildername:
    ct_page_type = '100k'
    num_pages = 100000
  elif '1m' in buildername:
    ct_page_type = 'All'
    num_pages = 1000000
  else:
    raise Exception('Do not recognise the buildername %s.' % buildername)

  # Figure out which tool to use.
  if 'DM' in buildername:
    skia_tool = 'dm'
    build_target = 'dm'
  elif 'BENCH' in buildername:
    skia_tool = 'nanobench'
    build_target = 'nanobench'
  elif 'IMG_DECODE' in buildername:
    skia_tool = 'get_images_from_skps'
    build_target = 'tools'
  else:
    raise Exception('Do not recognise the buildername %s.' % buildername)

  api.core.setup()
  api.flavor.compile(build_target)

  # Required paths.
  infrabots_dir = api.vars.skia_dir.join('infra', 'bots')
  isolate_dir = infrabots_dir.join('ct')
  isolate_path = isolate_dir.join(CT_SKPS_ISOLATE)

  api.run.copy_build_products(
      api.flavor.out_dir,
      isolate_dir)
  api.skia_swarming.setup(
      infrabots_dir.join('tools', 'luci-go'),
      swarming_rev='')

  skps_chromium_build = api.properties.get(
      'skps_chromium_build', DEFAULT_SKPS_CHROMIUM_BUILD)

  # Set build properties to make finding SKPs convenient.
  webpage_rankings_link = (
      'https://storage.cloud.google.com/%s/csv/top-1m.csv'
          % api.ct.CT_GS_BUCKET)
  api.step.active_result.presentation.properties['Webpage rankings'] = (
      webpage_rankings_link)
  download_skps_link = (
      'https://pantheon.corp.google.com/storage/browser/%s/swarming/skps/%s/%s/'
          % (api.ct.CT_GS_BUCKET, ct_page_type, skps_chromium_build))
  api.step.active_result.presentation.properties['Download SKPs by rank'] = (
      download_skps_link)

  # Delete swarming_temp_dir to ensure it starts from a clean slate.
  api.run.rmtree(api.skia_swarming.swarming_temp_dir)

  num_per_slave = api.properties.get(
      'num_per_slave',
      min(TOOL_TO_DEFAULT_SKPS_PER_SLAVE[skia_tool], num_pages))
  ct_num_slaves = api.properties.get(
      'ct_num_slaves',
      int(math.ceil(float(num_pages) / num_per_slave)))

  # Try to figure out if the SKPs we are going to isolate already exist
  # locally by reading the SKPS_VERSION_FILE.
  download_skps = True
  expected_version_contents = {
      "chromium_build": skps_chromium_build,
      "page_type": ct_page_type,
      "num_slaves": ct_num_slaves,
  }
  skps_dir = api.vars.checkout_root.join('skps', buildername)
  version_file = skps_dir.join(SKPS_VERSION_FILE)
  if api.path.exists(version_file):  # pragma: nocover
    version_file_contents = api.file.read_text(
        "Read %s" % version_file,
        version_file,
        test_data=expected_version_contents)
    actual_version_contents = api.json.loads(version_file_contents)
    differences = (set(expected_version_contents.items()) ^
                   set(actual_version_contents.items()))
    download_skps = len(differences) != 0
    if download_skps:
      # Delete and recreate the skps dir.
      api.run.rmtree(skps_dir)
      api.file.ensure_directory(
          'makedirs %s' % api.path.basename(skps_dir), skps_dir)

  # If a blacklist file exists then specify SKPs to be blacklisted.
  blacklists_dir = api.vars.skia_dir.join('infra', 'bots', 'ct', 'blacklists')
  blacklist_file = blacklists_dir.join(
      '%s_%s_%s.json' % (skia_tool, ct_page_type, skps_chromium_build))
  blacklist_skps = []
  if api.path.exists(blacklist_file):  # pragma: nocover
    blacklist_file_contents = api.file.read_text(
        "Read %s" % blacklist_file,
        blacklist_file)
    blacklist_skps = api.json.loads(blacklist_file_contents)['blacklisted_skps']

  for slave_num in range(1, ct_num_slaves + 1):
    if download_skps:
      # Download SKPs.
      api.ct.download_swarming_skps(
          ct_page_type, slave_num, skps_chromium_build,
          skps_dir,
          start_range=((slave_num-1)*num_per_slave) + 1,
          num_skps=num_per_slave)

    # Create this slave's isolated.gen.json file to use for batcharchiving.
    extra_variables = {
        'SLAVE_NUM': str(slave_num),
        'TOOL_NAME': skia_tool,
        'GIT_HASH': api.vars.got_revision,
        'CONFIGURATION': api.vars.configuration,
        'BUILDER': buildername,
    }
    api.skia_swarming.create_isolated_gen_json(
        isolate_path, isolate_dir, 'linux', 'ct-%s-%s' % (skia_tool, slave_num),
        extra_variables, blacklist=blacklist_skps)

  if download_skps:
    # Since we had to download SKPs create an updated version file.
    api.file.write_text("Create %s" % version_file,
                        version_file,
                        api.json.dumps(expected_version_contents))

  # Batcharchive everything on the isolate server for efficiency.
  max_slaves_to_batcharchive = MAX_SLAVES_TO_BATCHARCHIVE
  if '1m' in buildername:
    # Break up the "isolate tests" step into batches with <100k files due to
    # https://github.com/luci/luci-go/issues/9
    max_slaves_to_batcharchive = 5
  tasks_to_swarm_hashes = []
  for slave_start_num in xrange(1, ct_num_slaves+1, max_slaves_to_batcharchive):
    m = min(max_slaves_to_batcharchive, ct_num_slaves)
    batcharchive_output = api.skia_swarming.batcharchive(
        targets=['ct-' + skia_tool + '-%s' % num for num in range(
            slave_start_num, slave_start_num + m)])
    tasks_to_swarm_hashes.extend(batcharchive_output)
  # Sort the list to go through tasks in order.
  tasks_to_swarm_hashes.sort()

  # Trigger all swarming tasks.
  dimensions={'os': 'Ubuntu-14.04', 'cpu': 'x86-64', 'pool': 'Chrome'}
  if 'GPU' in buildername:
    dimensions['gpu'] = '10de:104a'
  tasks = api.skia_swarming.trigger_swarming_tasks(
      tasks_to_swarm_hashes, dimensions=dimensions, io_timeout=40*60)

  # Now collect all tasks.
  env = {'AWS_CREDENTIAL_FILE': None, 'BOTO_CONFIG': None}
  failed_tasks = []
  for task in tasks:
    try:
      api.skia_swarming.collect_swarming_task(task)

      if skia_tool == 'nanobench':
        output_dir = api.skia_swarming.tasks_output_dir.join(
            task.title).join('0')
        utc = api.time.utcnow()
        gs_dest_dir = 'ct/%s/%d/%02d/%02d/%02d/' % (
            ct_page_type, utc.year, utc.month, utc.day, utc.hour)
        for json_output in api.file.listdir(
            'listdir output dir', output_dir, test_data=['file 1', 'file 2']):
          with api.context(env=env):
            api.gsutil.upload(
                name='upload json output',
                source=json_output,
                bucket='skia-perf',
                dest=gs_dest_dir,
                args=['-R']
            )

    except api.step.StepFailure as e:
      # Add SKP links for convenience.
      api.step.active_result.presentation.links['Webpage rankings'] = (
          webpage_rankings_link)
      api.step.active_result.presentation.links['Download SKPs by rank'] = (
          download_skps_link)
      failed_tasks.append(e)

  if failed_tasks:
    raise api.step.StepFailure(
        'Failed steps: %s' % ', '.join([f.name for f in failed_tasks]))


def GenTests(api):
  ct_num_slaves = 5
  num_per_slave = 10
  skia_revision = 'abc123'
  path_config = 'kitchen'

  yield(
    api.test('CT_DM_10k_SKPs') +
    api.properties(
        buildername='Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_DM_10k_SKPs',
        path_config=path_config,
        swarm_out_dir='[SWARM_OUT_DIR]',
        ct_num_slaves=ct_num_slaves,
        num_per_slave=num_per_slave,
        repository='https://skia.googlesource.com/skia.git',
        revision=skia_revision,
    )
  )

  yield(
    api.test('CT_IMG_DECODE_10k_SKPs') +
    api.properties(
        buildername='Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_IMG_DECODE_'
                    '10k_SKPs',
        path_config=path_config,
        swarm_out_dir='[SWARM_OUT_DIR]',
        ct_num_slaves=ct_num_slaves,
        num_per_slave=num_per_slave,
        repository='https://skia.googlesource.com/skia.git',
        revision=skia_revision,
    )
  )

  yield(
    api.test('CT_DM_100k_SKPs') +
    api.properties(
        buildername='Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_DM_100k_SKPs',
        path_config=path_config,
        swarm_out_dir='[SWARM_OUT_DIR]',
        ct_num_slaves=ct_num_slaves,
        num_per_slave=num_per_slave,
        repository='https://skia.googlesource.com/skia.git',
        revision=skia_revision,
    )
  )

  yield(
    api.test('CT_IMG_DECODE_100k_SKPs') +
    api.properties(
        buildername='Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_IMG_DECODE_'
                    '100k_SKPs',
        path_config=path_config,
        swarm_out_dir='[SWARM_OUT_DIR]',
        ct_num_slaves=ct_num_slaves,
        num_per_slave=num_per_slave,
        repository='https://skia.googlesource.com/skia.git',
        revision=skia_revision,
    )
  )

  yield(
    api.test('CT_GPU_BENCH_1k_SKPs') +
    api.properties(
        buildername=
            'Perf-Ubuntu-GCC-Golo-GPU-GT610-x86_64-Release-CT_BENCH_1k_SKPs',
        path_config=path_config,
        swarm_out_dir='[SWARM_OUT_DIR]',
        ct_num_slaves=ct_num_slaves,
        num_per_slave=num_per_slave,
        repository='https://skia.googlesource.com/skia.git',
        revision=skia_revision,
    ) +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('src')
    )
  )

  yield(
    api.test('CT_CPU_BENCH_10k_SKPs') +
    api.properties(
        buildername=
            'Perf-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Release-CT_BENCH_10k_SKPs',
        path_config=path_config,
        swarm_out_dir='[SWARM_OUT_DIR]',
        ct_num_slaves=ct_num_slaves,
        num_per_slave=num_per_slave,
        repository='https://skia.googlesource.com/skia.git',
        revision=skia_revision,
    ) +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('src')
    )
  )

  yield(
    api.test('CT_GPU_BENCH_10k_SKPs') +
    api.properties(
        buildername=
            'Perf-Ubuntu-GCC-Golo-GPU-GT610-x86_64-Release-CT_BENCH_10k_SKPs',
        path_config=path_config,
        swarm_out_dir='[SWARM_OUT_DIR]',
        ct_num_slaves=ct_num_slaves,
        num_per_slave=num_per_slave,
        repository='https://skia.googlesource.com/skia.git',
        revision=skia_revision,
    ) +
    api.path.exists(
        api.path['start_dir'].join('skia'),
        api.path['start_dir'].join('src')
    )
  )

  yield(
    api.test('CT_DM_1m_SKPs') +
    api.properties(
        buildername='Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_DM_1m_SKPs',
        path_config=path_config,
        swarm_out_dir='[SWARM_OUT_DIR]',
        ct_num_slaves=ct_num_slaves,
        num_per_slave=num_per_slave,
        repository='https://skia.googlesource.com/skia.git',
        revision=skia_revision,
    )
  )

  yield (
    api.test('CT_DM_SKPs_UnknownBuilder') +
    api.properties(
        buildername=
            'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_DM_UnknownRepo_SKPs',
        path_config=path_config,
        swarm_out_dir='[SWARM_OUT_DIR]',
        ct_num_slaves=ct_num_slaves,
        num_per_slave=num_per_slave,
        repository='https://skia.googlesource.com/skia.git',
        revision=skia_revision,
    ) +
    api.expect_exception('Exception')
  )

  yield (
    api.test('CT_10k_SKPs_UnknownBuilder') +
    api.properties(
        buildername=
            'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_UnknownTool_10k_SKPs',
        path_config=path_config,
        swarm_out_dir='[SWARM_OUT_DIR]',
        ct_num_slaves=ct_num_slaves,
        num_per_slave=num_per_slave,
        repository='https://skia.googlesource.com/skia.git',
        revision=skia_revision,
    ) +
    api.expect_exception('Exception')
  )

  yield(
    api.test('CT_DM_1m_SKPs_slave3_failure') +
    api.step_data('ct-dm-3', retcode=1) +
    api.properties(
        buildername='Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_DM_1m_SKPs',
        path_config=path_config,
        swarm_out_dir='[SWARM_OUT_DIR]',
        ct_num_slaves=ct_num_slaves,
        num_per_slave=num_per_slave,
        repository='https://skia.googlesource.com/skia.git',
        revision=skia_revision,
    )
  )

  yield(
    api.test('CT_DM_1m_SKPs_2slaves_failure') +
    api.step_data('ct-dm-1', retcode=1) +
    api.step_data('ct-dm-3', retcode=1) +
    api.properties(
        buildername='Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_DM_1m_SKPs',
        path_config=path_config,
        swarm_out_dir='[SWARM_OUT_DIR]',
        ct_num_slaves=ct_num_slaves,
        num_per_slave=num_per_slave,
        repository='https://skia.googlesource.com/skia.git',
        revision=skia_revision,
    )
  )

  builder = 'Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_DM_10k_SKPs'
  yield(
    api.test('CT_DM_10k_SKPs_Trybot') +
    api.properties(
        buildername=builder,
        path_config=path_config,
        swarm_out_dir='[SWARM_OUT_DIR]',
        ct_num_slaves=ct_num_slaves,
        num_per_slave=num_per_slave,
        repository='https://skia.googlesource.com/skia.git',
        patch_storage='gerrit') +
    api.properties.tryserver(
        buildername=builder,
        gerrit_project='skia',
        gerrit_url='https://skia-review.googlesource.com/',
    )
  )

  builder = ('Test-Ubuntu-GCC-GCE-CPU-AVX2-x86_64-Debug-CT_IMG_DECODE_'
             '10k_SKPs')
  yield(
    api.test('CT_IMG_DECODE_10k_SKPs_Trybot') +
    api.properties(
        buildername=builder,
        path_config=path_config,
        swarm_out_dir='[SWARM_OUT_DIR]',
        ct_num_slaves=ct_num_slaves,
        num_per_slave=num_per_slave,
        repository='https://skia.googlesource.com/skia.git',
        revision=skia_revision,
        patch_storage='gerrit') +
    api.properties.tryserver(
        buildername=builder,
        gerrit_project='skia',
        gerrit_url='https://skia-review.googlesource.com/',
    )
  )
