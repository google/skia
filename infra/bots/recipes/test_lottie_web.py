# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which generates the Gold images for lottie-web using docker

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


DOCKER_IMAGE = 'gcr.io/skia-public/gold-lottie-web-puppeteer:v2'
LOTTIECAP_SCRIPT = '/SRC/skia/infra/lottiecap/docker/lottiecap_gold.sh'


def RunSteps(api):
  api.vars.setup()
  checkout_root = api.checkout.default_checkout_root
  out_dir = api.vars.swarming_out_dir
  lottie_files_src = api.vars.slave_dir.join('lottie-samples')
  lottie_files_dir = '/tmp/lottie_files'
  # The lottie-web repo is DEP'd in. This links to its build directory
  # to make finding the lottie.min.js easier to reference from inside
  # the docker image.
  lottie_build = checkout_root.join('lottie', 'build', 'player')

  api.checkout.bot_update(checkout_root=checkout_root)

  # Make sure this exists, otherwise Docker will make it with root permissions.
  api.file.ensure_directory('mkdirs out_dir', out_dir, mode=0777)
  # When lottie files are brought in via isolate or CIPD, they are just
  # symlinks, which does not work well with Docker because we can't mount
  # the folder as a volume.
  # e.g. /LOTTIE_FILES/foo.json -> ../.cipd/alpha/beta/foo.json
  # To get around this, we just copy the lottie files into a temporary
  # directory.
  api.file.rmtree('remove previous lottie files', lottie_files_dir)
  api.file.copytree('copy lottie files', lottie_files_src, lottie_files_dir)

  api.python.inline(
      name='Set up for docker',
      program='''
import os
import sys

lottie_files_dir = sys.argv[1]
out_dir = sys.argv[2]
lottie_build = sys.argv[3]

# Make sure all the lottie files are readable by everyone so we can see
# them in the docker container.
os.system('chmod 0644 %s/*' % lottie_files_dir)
os.system('chmod 0644 %s/*' % lottie_build)

# Prepare output folder, api.file.ensure_directory doesn't touch
# the permissions of the out directory if it already exists.
# This typically means that the non-privileged docker won't be able to write.
os.chmod(out_dir, 0o777)
''',
      args=[lottie_files_dir, out_dir, lottie_build],
      infra_step=True)

  cmd = ['docker', 'run', '--shm-size=2gb', '--rm',
         '-v', '%s:/SRC' % checkout_root,
         '-v', '%s:/OUT' % out_dir,
         '-v', '%s:/LOTTIE_BUILD' % lottie_build,
         '-v', '%s:/LOTTIE_FILES' % lottie_files_dir]

  cmd.extend([
         DOCKER_IMAGE,             LOTTIECAP_SCRIPT,
         '--builder',              api.vars.builder_name,
         '--git_hash',             api.properties['revision'],
         '--buildbucket_build_id', api.properties.get('buildbucket_build_id',
                                                      ''),
         '--bot_id',               api.vars.swarming_bot_id,
         '--task_id',              api.vars.swarming_task_id,
         '--browser',              'Chrome',
         '--config',               api.vars.configuration,
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
        'Create lottie-web Gold output with Docker',
        cmd=cmd)


def GenTests(api):
  yield (
      api.test('Test-Debian9-none-GCE-CPU-AVX2-x86_64-Debug-All-LottieWeb') +
      api.properties(buildername=('Test-Debian9-none-GCE-CPU-AVX2'
                                  '-x86_64-Debug-All-LottieWeb'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('lottie_web_trybot') +
      api.properties(buildername=('Test-Debian9-none-GCE-CPU-AVX2'
                                  '-x86_64-Debug-All-LottieWeb'),
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
