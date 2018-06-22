# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe module for Skia Swarming SKQP testing.


DEPS = [
  'flavor',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'run',
  'vars',
]


def test_firebase_steps(api):
  """Test an APK on Firebase Testlab."""
  wlist_file = api.vars.slave_dir.join('whitelist_devices.json')
  apk_file = api.vars.slave_dir.join('out','devrel','skqp-universal-debug.apk')
  upload_path = 'skia-stephana-test/testing/skqp-universal-debug.apk'
  args = [
    'run_testlab',
    '--logtostderr',
    '--devices', wlist_file,
    '--upload_path', upload_path,
    apk_file
  ]
  api.run(api.flavor.step, 'run firebase testlab', cmd=args)


def RunSteps(api):
  api.vars.setup()
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)

  os = api.properties['os']
  compiler = api.properties['compiler']
  model = api.properties['model']
  cpu_or_gpu = api.properties['cpu_or_gpu']
  cpu_or_gpu_value = api.properties['cpu_or_gpu_value']
  arch = api.properties['arch']
  configuration = api.properties['configuration']
  test_filter = api.properties['test_filter']
  extra_tokens = api.properties.get('extra_tokens', '').split(',')
  api.flavor.setup(os, compiler, model, cpu_or_gpu, cpu_or_gpu_value, arch,
                   configuration, test_filter, extra_tokens)

  test_firebase_steps(api)
  api.run.check_failure()


# Default properties used for TEST_BUILDERS.
def defaultProps(buildername):
  split = buildername.split('-')
  os = split[1]
  compiler = split[2]
  model = split[3]
  cpu_or_gpu = split[4]
  cpu_or_gpu_value = split[5]
  arch = split[6]
  configuration = split[7]
  test_filter = split[8]

  extra_tokens_list = []
  if len(split) > 9:
    extra_split = split[9].split('_')
    for idx, tok in enumerate(extra_split):
      if tok == 'SK':  # pragma: no cover
        extra_tokens_list.append('_'.join(extra_split[idx:]))
        break
      else:
        extra_tokens_list.append(tok)
  extra_tokens = ','.join(extra_tokens_list)

  return dict(
    arch=arch,
    buildername=buildername,
    buildbucket_build_id='123454321',
    compiler=compiler,
    configuration=configuration,
    cpu_or_gpu=cpu_or_gpu,
    cpu_or_gpu_value=cpu_or_gpu_value,
    extra_tokens=extra_tokens,
    model=model,
    os=os,
    revision='abc123',
    path_config='kitchen',
    swarm_out_dir='[SWARM_OUT_DIR]',
    test_filter=test_filter,
  )


def GenTests(api):
  builder = 'Test-Debian9-Clang-GCE-CPU-AVX2-universal-devrel-All-Android_SKQP'
  yield (
      api.test(builder) +
      api.properties(**defaultProps(builder))
  )
