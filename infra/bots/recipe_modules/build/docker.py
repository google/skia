# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import util


def compile_fn(api, checkout_root, out_dir):
  skia_dir = checkout_root.join('skia')
  compiler = api.vars.builder_cfg.get('compiler', '')
  configuration = api.vars.builder_cfg.get('configuration', '')
  extra_tokens = api.vars.extra_tokens
  extra_tokens.remove('Docker')
  os = api.vars.builder_cfg.get('os', '')
  target_arch = api.vars.builder_cfg.get('target_arch', '')

  args = {
      'target_cpu': '"%s"' % target_arch,
      'werror': 'true'
  }
  if configuration != 'Debug':
    args['is_debug'] = 'false'

  image = None
  if os == 'Debian10':
    if compiler == 'GCC':
      if not extra_tokens:
        image = (
            'gcr.io/skia-public/gcc-debian10@sha256:'
            'fb04357a54c34ab07646a0d9defd8d44a180c03edb6b00f9cb7220fdad69a58f')
        args['cc'] = '"gcc"'
        args['cxx'] = '"g++"'
  if not image:
    raise Exception('Not implemented: ' + api.vars.builder_name)

  # We always perform an incremental compile, since out dir is cached across
  # compile tasks. However, we need to force a recompile when the toolchain
  # changes. The simplest way to do that is using a C define that changes
  # anytime the image changes.
  args['extra_cflags'] = '["-DDUMMY_docker_image=%s"]' % image

  # We want to make sure the directories exist and were created by chrome-bot.
  # (Note that the docker --mount option, unlike the --volume option, does not
  # create this dir as root if it doesn't exist.)
  api.file.ensure_directory('mkdirs out_dir', out_dir, mode=0777)

  # It's easiest to run GN outside the Docker container.
  gn_args = ' '.join('%s=%s' % (k, v) for (k, v) in sorted(args.iteritems()))
  gn = skia_dir.join('bin', 'gn')
  with api.context(cwd=skia_dir):
    api.run(api.python,
            'fetch-gn',
            script=skia_dir.join('bin', 'fetch-gn'),
            infra_step=True)
    api.run(api.step, 'gn gen',
            cmd=[gn, 'gen', out_dir, '--args=' + gn_args])

  # Run ninja inside the docker container. It binds two folders: the Skia
  # checkout to /SRC and the output directory to /OUT.
  src_mnt = 'type=bind,source=%s,target=/SRC' % checkout_root
  out_mnt = 'type=bind,source=%s,target=/OUT' % out_dir
  depot_tools_mnt = 'type=bind,source=%s,target=/depot_tools' % (
      api.path['start_dir'].join('recipe_bundle', 'depot_tools'))
  cmd = ['docker', 'run', '--rm', '--mount', src_mnt, '--mount', out_mnt,
         '--mount', depot_tools_mnt, image, '/depot_tools/ninja', '-C', out_dir]
  # Override DOCKER_CONFIG set by Kitchen.
  env = {'DOCKER_CONFIG': '/home/chrome-bot/.docker'}
  with api.env(env):
    api.run(
        api.step,
        'Run ninja in Docker',
        cmd=cmd)

def copy_build_products(api, src, dst):
  util.copy_listed_files(api, src, dst, util.DEFAULT_BUILD_PRODUCTS)
