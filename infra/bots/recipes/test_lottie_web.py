# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which generates the Gold images for lottie-web using docker


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


DOCKER_IMAGE = 'gcr.io/skia-public/gold-lottie-web-puppeteer:5.2.1_v1'
INNER_KARMA_SCRIPT = '/SRC/skia/infra/lottiecap/docker/lottiecap_gold.sh'



def RunSteps(api):
  api.vars.setup()
  checkout_root = api.checkout.default_checkout_root
  out_dir = api.vars.swarming_out_dir
  lottie_files_dir = api.vars.slave_dir.join('lottie-samples')
  api.checkout.bot_update(checkout_root=checkout_root)

  # Make sure this exists, otherwise Docker will make it with root permissions.
  api.file.ensure_directory('mkdirs out_dir', out_dir, mode=0777)


  cmd = ['docker', 'run', '--shm-size=2gb', '--rm',
         '-v', '%s:/SRC' % checkout_root,
         '-v', '%s:/OUT' % out_dir,
         '-v', '%s:/LOTTIE_FILES' % lottie_files_dir
         ]

  cmd.extend([
         DOCKER_IMAGE,  INNER_KARMA_SCRIPT,
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
      api.test('pathkit_trybot') +
      api.properties(buildername=('Test-Debian9-none-GCE-CPU-AVX2'
                                  '-x86_64-Debug-All-LottieWeb'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]',
                     patch_storage='gerrit',
                     patch_set=7,
                     patch_issue=1234,
                     gerrit_project='skia',
                     gerrit_url='https://skia-review.googlesource.com/')
  )
