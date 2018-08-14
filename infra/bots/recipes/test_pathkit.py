# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the PathKit tests using docker


DEPS = [
  'checkout',
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


DOCKER_IMAGE = 'gcr.io/skia-public/karma-chrome-tests:68.0.3440.106_v2'
INNER_KARMA_SCRIPT = '/SRC/skia/experimental/pathkit/docker/test_pathkit.sh'



def RunSteps(api):
  api.vars.setup()
  checkout_root = api.checkout.default_checkout_root
  out_dir = api.vars.swarming_out_dir
  api.checkout.bot_update(checkout_root=checkout_root)

  # Make sure this exists, otherwise Docker will make it with root permissions.
  api.file.rmtree('clean out_dir', out_dir)
  api.file.ensure_directory('mkdirs out_dir', out_dir, mode=0777)

  copy_dest = api.path.join(checkout_root, 'skia', 'experimental', 'pathkit',
                        'npm-wasm', 'bin', 'test')

  helper_js = api.vars.build_dir.join('pathkit.js')
  wasm = api.vars.build_dir.join('pathkit.wasm')

  api.python.inline(
      name='copy built wasm to location docker can see',
      program='''import errno
import os
import shutil
import sys

copy_dest = sys.argv[1]
helper_js = sys.argv[2]
wasm = sys.argv[3]

# Clean out old binaries (if any)
try:
  shutil.rmtree(copy_dest)
except OSError as e:
  if e.errno != errno.ENOENT:
    raise

# Make folder
try:
  os.makedirs(copy_dest)
except OSError as e:
  if e.errno != errno.EEXIST:
    raise

dest = os.path.join(copy_dest, 'pathkit.js')
shutil.copyfile(helper_js, dest)
os.chmod(dest, 0o644) # important, otherwise non-privileged docker can't read.

dest = os.path.join(copy_dest, 'pathkit.wasm')
shutil.copyfile(wasm, dest)
os.chmod(dest, 0o644) # important, otherwise non-privileged docker can't read.
''',
      args=[copy_dest, helper_js, wasm],
      infra_step=True)



  cmd = ['docker', 'run', '--shm-size=2gb', '--rm',
         '-v', '%s:/SRC' % checkout_root, '-v', '%s:/OUT' % out_dir,
         DOCKER_IMAGE,  INNER_KARMA_SCRIPT,
         '--builder',              api.vars.builder_name,
         '--git_hash',             api.properties['revision'],
         '--buildbucket_build_id', api.properties.get('buildbucket_build_id',
                                                      ''),
         '--bot_id',               api.vars.swarming_bot_id,
         '--task_id',              api.vars.swarming_task_id,
         '--browser',              'Chrome',
         '--config',               api.vars.configuration,
         ]

  api.run(
    api.step,
    'Test PathKit with Docker',
    cmd=cmd)


def GenTests(api):
  yield (
      api.test('pathkit_test') +
      api.properties(buildername=('Test-Debian9-EMCC-GCE-CPU-AVX2'
                                  '-wasm-Debug-All-PathKit'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )
