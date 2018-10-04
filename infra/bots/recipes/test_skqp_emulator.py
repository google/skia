# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which runs the SKQP apk using docker and an Android Emulator

DEPS = [
  'checkout',
  'infra',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
  'run',
  'vars',
]


DOCKER_IMAGE = 'gcr.io/skia-public/android-skqp:8.1_v1'
INNER_KARMA_SCRIPT = '/SRC/skia/infra/skqp/run_skqp.sh'


def RunSteps(api):
  api.vars.setup()
  checkout_root = api.checkout.default_checkout_root
  api.checkout.bot_update(checkout_root=checkout_root)

  # This is where the APK should be
  base_dir = api.vars.build_dir

  container_name = 'android_em'

  api.run(
    api.step,
    'Start Emulator',
    cmd=['docker', 'run', '--privileged', '--rm', '-d', # detached/daemon
         '--name', container_name,
         '--env', 'DEVICE="Samsung Galaxy S6"',
         '--volume', '%s:/SRC' % checkout_root,
         '--volume', '%s:/OUT' % base_dir,
         DOCKER_IMAGE],
    infra_step=True)

  api.run(
    api.step,
    'Test SQKP with Android Emulator in Docker',
    cmd=['docker', 'exec', '-it', container_name,
         DOCKER_IMAGE, INNER_KARMA_SCRIPT])

  api.run(
    api.step,
    'Stop Emulator',
    cmd=['docker', 'kill', container_name],
    infra_step=True)


def GenTests(api):
  yield (
      api.test('Test-Debian9-Clang-GCE-CPU-Emulator-x86-devrel'
               '-All-Android_SKQP') +
      api.properties(buildername=('Test-Debian9-Clang-GCE-CPU-Emulator'
                                  '-x86-devrel-All-Android_SKQP'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
