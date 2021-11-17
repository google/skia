# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming test.


import json

PYTHON_VERSION_COMPATIBILITY = "PY2+3"

DEPS = [
  'env',
  'flavor',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'gold_upload',
  'run',
  'vars',
]

DM_JSON = 'dm.json'

def test_steps(api):
  """Run the DM test."""
  do_upload = api.properties.get('do_upload') == 'true'
  images = api.properties.get('images') == 'true'
  lotties = api.properties.get('lotties') == 'true'
  resources = api.properties.get('resources') == 'true'
  skps = api.properties.get('skps') == 'true'
  svgs = api.properties.get('svgs') == 'true'

  api.flavor.install(
      images=images,
      lotties=lotties,
      resources=resources,
      skps=skps,
      svgs=svgs,
  )

  use_hash_file = False
  if do_upload:
    host_dm_dir = str(api.flavor.host_dirs.dm_dir)
    api.flavor.create_clean_host_dir(api.path['start_dir'].join('test'))
    device_dm_dir = str(api.flavor.device_dirs.dm_dir)
    if host_dm_dir != device_dm_dir:
      api.flavor.create_clean_device_dir(device_dm_dir)

    # Obtain the list of already-generated hashes.
    hash_filename = 'uninteresting_hashes.txt'

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

        HASHES_URL = sys.argv[1]
        RETRIES = 5
        TIMEOUT = 60
        WAIT_BASE = 15

        socket.setdefaulttimeout(TIMEOUT)
        for retry in range(RETRIES):
          try:
            with contextlib.closing(
                urllib2.urlopen(HASHES_URL, timeout=TIMEOUT)) as w:
              hashes = w.read()
              with open(sys.argv[2], 'w') as f:
                f.write(hashes)
                break
          except Exception as e:
            print('Failed to get uninteresting hashes from %s:' % HASHES_URL)
            print(e)
            if retry == RETRIES:
              raise
            waittime = WAIT_BASE * math.pow(2, retry)
            print('Retry in %d seconds.' % waittime)
            time.sleep(waittime)
        """,
        args=[api.properties['gold_hashes_url'], host_hashes_file],
        abort_on_failure=False,
        fail_build_on_failure=False,
        infra_step=True)

    if api.path.exists(host_hashes_file):
      api.flavor.copy_file_to_device(host_hashes_file, hashes_file)
      use_hash_file = True

  # Find DM flags.
  args = json.loads(api.properties['dm_flags'])
  props = json.loads(api.properties['dm_properties'])
  args.append('--properties')
  # Map iteration order is arbitrary; in order to maintain a consistent step
  # ordering, sort by key.
  for k in sorted(props.keys()):
    v = props[k]
    if v == '${SWARMING_BOT_ID}':
      v = api.vars.swarming_bot_id
    elif v == '${SWARMING_TASK_ID}':
      v = api.vars.swarming_task_id
    if v != '':
      args.extend([k, v])

  # Paths to required resources.
  if resources:
    args.extend(['--resourcePath', api.flavor.device_dirs.resource_dir])
  if skps:
    args.extend(['--skps', api.flavor.device_dirs.skp_dir])
  if images:
    args.extend([
        '--images', api.flavor.device_path_join(
            api.flavor.device_dirs.images_dir, 'dm'),
        '--colorImages', api.flavor.device_path_join(
            api.flavor.device_dirs.images_dir, 'colorspace'),
    ])
  if svgs:
    # svg_dir is the root of the SVG corpus. Within that directory,
    # the *.svg inputs are in the 'svg' subdirectory. See skbug.com/11229
    args.extend(['--svgs', api.flavor.device_path_join(
      api.flavor.device_dirs.svg_dir, "svg")])
  if lotties:
    args.extend([
      '--lotties',
      api.flavor.device_path_join(
          api.flavor.device_dirs.resource_dir, 'skottie'),
      api.flavor.device_dirs.lotties_dir,
    ])

  if use_hash_file:
    args.extend(['--uninterestingHashesFile', hashes_file])
  if do_upload:
    args.extend(['--writePath', api.flavor.device_dirs.dm_dir])

  # Run DM.
  api.run(api.flavor.step, 'dm', cmd=args, abort_on_failure=False)

  if do_upload:
    # Copy images and JSON to host machine if needed.
    api.flavor.copy_directory_contents_to_host(
        api.flavor.device_dirs.dm_dir, api.flavor.host_dirs.dm_dir)
    # https://bugs.chromium.org/p/chromium/issues/detail?id=1192611
    if 'Win' not in api.vars.builder_cfg.get('os', ''):
      api.gold_upload.upload()


def RunSteps(api):
  api.vars.setup()
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)
  api.flavor.setup('dm')

  try:
    test_steps(api)
  finally:
    api.flavor.cleanup_steps()
  api.run.check_failure()


TEST_BUILDERS = [
  'Test-Android-Clang-Pixel2XL-GPU-Adreno540-arm-Debug-All-Android_ASAN',
  'Test-Android-Clang-Pixel2XL-GPU-Adreno540-arm64-Debug-All-Android',
  'Test-Debian10-Clang-GCE-CPU-AVX2-x86_64-Release-All-Lottie',
  'Test-Win10-Clang-ShuttleC-GPU-GTX960-x86_64-Debug-All-ANGLE',
]


def GenTests(api):
  for builder in TEST_BUILDERS:
    props = dict(
      buildername=builder,
      buildbucket_build_id='123454321',
      dm_flags='["dm","--example","--flags"]',
      dm_properties=('{"key1":"value1","key2":"",'
                     '"bot":"${SWARMING_BOT_ID}",'
                     '"task":"${SWARMING_TASK_ID}"}'),
      revision='abc123',
      gs_bucket='skia-infra-gm',
      patch_ref='89/456789/12',
      patch_set=7,
      patch_issue=1234,
      path_config='kitchen',
      gold_hashes_url='https://example.com/hashes.txt',
      swarm_out_dir='[SWARM_OUT_DIR]',
      task_id='task_12345',
      resources='true',
    )
    if 'ASAN' not in builder:
      props['do_upload'] = 'true'
    if 'Lottie' in builder:
      props['lotties'] = 'true'
    else:
      props['images'] = 'true'
      props['skps'] = 'true'
      props['svgs'] = 'true'
    test = (
      api.test(builder) +
      api.properties(**props) +
      api.path.exists(
          api.path['start_dir'].join('skia'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skimage', 'VERSION'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'skp', 'VERSION'),
          api.path['start_dir'].join('skia', 'infra', 'bots', 'assets',
                                     'svg', 'VERSION'),
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      ) +
      api.step_data('get swarming bot id',
          stdout=api.raw_io.output('skia-bot-123')) +
      api.step_data('get swarming task id',
          stdout=api.raw_io.output('123456'))
    )
    if 'Win' in builder:
      test += api.platform('win', 64)

    yield test
