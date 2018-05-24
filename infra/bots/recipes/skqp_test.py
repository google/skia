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
  api.flavor.setup()

  test_firebase_steps(api)
  api.run.check_failure()

def GenTests(api):
  builder = 'Test-Debian9-Clang-GCE-CPU-AVX2-universal-devrel-All-Android_SKQP'
  yield (
      api.test(builder) +
      api.properties(buildername=builder,
                     buildbucket_build_id='123454321',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
