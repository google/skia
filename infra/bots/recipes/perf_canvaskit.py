# Copyright 2018 The Chromium Authors. All rights reserved.
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


DOCKER_IMAGE = 'gcr.io/skia-public/perf-karma-chrome-tests:77.0.3865.120_v1'
INNER_KARMA_SCRIPT = '/SRC/skia/infra/canvaskit/perf_canvaskit.sh'


def RunSteps(api):
  api.vars.setup()
  checkout_root = api.checkout.default_checkout_root
  out_dir = api.vars.swarming_out_dir
  api.checkout.bot_update(checkout_root=checkout_root)

  # Make sure this exists, otherwise Docker will make it with root permissions.
  api.file.ensure_directory('mkdirs out_dir', out_dir, mode=0777)

  # The karma script is configured to look in ./canvaskit/bin/ for
  # the test files to load, so we must copy them there (see Set up for docker).
  copy_dest = checkout_root.join('skia', 'modules', 'canvaskit',
                                 'canvaskit', 'bin')

  base_dir = api.vars.build_dir
  bundle_name = 'canvaskit.wasm'

  api.python.inline(
      name='Set up for docker',
      program='''import errno
import os
import shutil
import sys

copy_dest = sys.argv[1]
base_dir = sys.argv[2]
bundle_name = sys.argv[3]
out_dir = sys.argv[4]

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

# Copy binaries (canvaskit.js and canvaskit.wasm) to where the karma tests
# expect them ($SKIA_ROOT/modules/canvaskit/canvaskit/bin/)
dest = os.path.join(copy_dest, 'canvaskit.js')
shutil.copyfile(os.path.join(base_dir, 'canvaskit.js'), dest)
os.chmod(dest, 0o644) # important, otherwise non-privileged docker can't read.

if bundle_name:
  dest = os.path.join(copy_dest, bundle_name)
  shutil.copyfile(os.path.join(base_dir, bundle_name), dest)
  os.chmod(dest, 0o644) # important, otherwise non-privileged docker can't read.

# Prepare output folder, api.file.ensure_directory doesn't touch
# the permissions of the out directory if it already exists.
os.chmod(out_dir, 0o777) # important, otherwise non-privileged docker can't write.
''',
      args=[copy_dest, base_dir, bundle_name, out_dir],
      infra_step=True)


  cmd = ['docker', 'run', '--shm-size=2gb', '--rm',
         '--volume', '%s:/SRC' % checkout_root,
         '--volume', '%s:/OUT' % out_dir]

  cmd.extend([
    DOCKER_IMAGE,             INNER_KARMA_SCRIPT,
    '--builder',              api.vars.builder_name,
    '--git_hash',             api.properties['revision'],
    '--buildbucket_build_id', api.properties.get('buildbucket_build_id',
                                              ''),
    '--bot_id',               api.vars.swarming_bot_id,
    '--task_id',              api.vars.swarming_task_id,
    '--browser',              'Chrome',
    '--config',               api.vars.configuration,
    '--source_type',          'canvaskit',
    ])

  if api.vars.is_trybot:
    cmd.extend([
      '--issue',         api.vars.issue,
      '--patchset',      api.vars.patchset,
      '--patch_storage', api.vars.patch_storage,
    ])

  # Override DOCKER_CONFIG set by Kitchen.
  env = {'DOCKER_CONFIG': '/home/chrome-bot/.docker'}
  with api.env(env):
    api.run(
        api.step,
        'Performance tests of canvaskit with Docker',
        cmd=cmd)


def GenTests(api):
  yield (
      api.test('Perf-Debian9-EMCC-GCE-CPU-AVX2-wasm-Release-All-CanvasKit') +
      api.properties(buildername=('Perf-Debian9-EMCC-GCE-CPU-AVX2'
                                  '-wasm-Release-All-CanvasKit'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('pathkit_trybot') +
      api.properties(buildername=('Perf-Debian9-EMCC-GCE-GPU-AVX2'
                                  '-wasm-Release-All-CanvasKit'),
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
