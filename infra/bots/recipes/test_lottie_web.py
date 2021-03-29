# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which generates the Gold images for lottie-web using docker

DEPS = [
  'checkout',
  'docker',
  'env',
  'flavor',
  'gold_upload',
  'infra',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'run',
  'vars',
]


DOCKER_IMAGE = 'gcr.io/skia-public/gold-lottie-web-puppeteer:v2'
LOTTIECAP_SCRIPT = 'skia/infra/lottiecap/docker/lottiecap_gold.sh'


def RunSteps(api):
  api.vars.setup()
  api.flavor.setup("dm")
  checkout_root = api.path['start_dir']
  out_dir = api.vars.swarming_out_dir
  lottie_files_src = api.vars.workdir.join('lottie-samples')
  lottie_files_dir = '/tmp/lottie_files'
  # The lottie-web repo is DEP'd in. This links to its build directory
  # to make finding the lottie.min.js easier to reference from inside
  # the docker image.
  lottie_build = checkout_root.join('lottie', 'build', 'player')

  # Make sure this exists, otherwise Docker will make it with root permissions.
  api.file.ensure_directory('mkdirs out_dir', out_dir, mode=0o777)
  # When lottie files are brought in via isolate or CIPD, they are just
  # symlinks, which does not work well with Docker because we can't mount
  # the folder as a volume.
  # e.g. /LOTTIE_FILES/foo.json -> ../.cipd/alpha/beta/foo.json
  # To get around this, we just copy the lottie files into a temporary
  # directory.
  api.file.rmtree('remove previous lottie files', lottie_files_dir)
  api.file.copytree('copy lottie files', lottie_files_src, lottie_files_dir)

  recursive_read = [lottie_build, lottie_files_dir]

  docker_args = [
      '--mount',
      'type=bind,source=%s,target=/LOTTIE_BUILD' % lottie_build,
      '--mount',
      'type=bind,source=%s,target=/LOTTIE_FILES' % lottie_files_dir
  ]

  args = [
    '--builder',              api.vars.builder_name,
    '--git_hash',             api.properties['revision'],
    '--buildbucket_build_id', api.properties.get('buildbucket_build_id',
                                                 ''),
    '--bot_id',               api.vars.swarming_bot_id,
    '--task_id',              api.vars.swarming_task_id,
    '--browser',              'Chrome',
    '--config',               api.vars.configuration,
  ]

  if api.vars.is_trybot:
    args.extend([
      '--issue',         api.vars.issue,
      '--patchset',      api.vars.patchset,
      '--patch_storage', api.vars.patch_storage,
    ])

  api.docker.run(
      name='Generate LottieWeb Gold output with Docker',
      docker_image=DOCKER_IMAGE,
      src_dir=checkout_root,
      out_dir=out_dir,
      script=checkout_root.join(LOTTIECAP_SCRIPT),
      args=args,
      docker_args=docker_args,
      recursive_read=recursive_read,
      attempts=3,
  )

  api.gold_upload.upload()


def GenTests(api):
  yield (
      api.test('Test-Debian10-none-GCE-CPU-AVX2-x86_64-Debug-All-LottieWeb') +
      api.properties(buildername=('Test-Debian10-none-GCE-CPU-AVX2'
                                  '-x86_64-Debug-All-LottieWeb'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     gs_bucket='skia-infra-gm',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('lottie_web_trybot') +
      api.properties(buildername=('Test-Debian10-none-GCE-CPU-AVX2'
                                  '-x86_64-Debug-All-LottieWeb'),
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     gs_bucket='skia-infra-gm',
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
