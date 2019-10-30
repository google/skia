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

# This image is public, and thus doesn't require log-in to read.
DOCKER_IMAGE = ('butomo1989/docker-android-x86-8.1@sha256:'
    'ad75c888e373d9ea7a2821fd8f64b53c9a22b5827e6fa516b396739a20b9bb88')
INNER_TEST_SCRIPT = '/SRC/skia/infra/skqp/run_skqp.sh'


def RunSteps(api):
  api.vars.setup()
  checkout_root = api.checkout.default_checkout_root
  api.checkout.bot_update(checkout_root=checkout_root)

  # This is where the APK should be, that is, where Swarming puts the inputs.
  apk_location = api.vars.build_dir

  container_name = 'android_em'

  # Make sure the emulator starts up, with some resilence against
  # occasional flakes.
  api.python.inline(
      name='Start Emulator',
      program='''
import os
import subprocess
import sys

container_name = sys.argv[1]
checkout_root = sys.argv[2]
apk_location = sys.argv[3]
DOCKER_IMAGE = sys.argv[4]

MAX_TRIES = 5

start_cmd = ['docker', 'run', '--privileged', '--rm', '-d', # detached/daemon
             '--name', container_name,
             '--env', 'DEVICE=Samsung Galaxy S6',
             '--volume', '%s:/SRC' % checkout_root,
             '--volume', '%s:/OUT' % apk_location,
             DOCKER_IMAGE]

wait_cmd = ['docker', 'exec', container_name,
            'timeout', '45', 'adb', 'wait-for-device']

for t in range(MAX_TRIES):
  print 'Starting Emulator try %d' % t
  try:
    # Start emulator
    print subprocess.check_output(start_cmd)
    # Wait a short time using adb-wait-for-device
    print subprocess.check_output(wait_cmd)
    # if exit code 0, we are good so end loop
    print 'Emulator started'
    sys.exit(0)
  except subprocess.CalledProcessError:
    # else kill docker container
    print 'Killing and trying again'
    print subprocess.check_output(['docker', 'kill', container_name])
print 'Could not start emulator'
sys.exit(1)
''',
      args=[container_name, checkout_root, apk_location, DOCKER_IMAGE],
      infra_step=True)

  api.run(
    api.step,
    'Test SQKP with Android Emulator in Docker',
    cmd=['docker', 'exec', container_name,
         INNER_TEST_SCRIPT])

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
