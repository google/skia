# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import util


def compile_fn(api, checkout_root, out_dir):
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
            '89a72df1e2fdea6f774a3fa4199bb9aaa4a0526a3ac1f233e604d689b694f95c')
        args['cc'] = '"gcc"'
        args['cxx'] = '"g++"'
  if not image:
    raise Exception('Not implemented: ' + api.vars.builder_name)

  # We always perform an incremental compile, since out dir is cached across
  # compile tasks. However, we need to force a recompile when the toolchain
  # changes. The simplest way to do that is using a C define that changes
  # anytime the image changes.
  args['extra_cflags'] = '["-DDUMMY_docker_image=%s"]' % image

  # Format the GN args for this build.
  gn_args = ' '.join('%s=%s' % (k, v) for (k, v) in sorted(args.iteritems()))
  api.docker.run('Run build script in Docker', image, checkout_root, out_dir,
                 'recipe_bundle/skia/infra/bots/recipe_modules'
                 '/build/resources/docker-compile.sh',
                 args=[gn_args])

def copy_build_products(api, src, dst):
  util.copy_listed_files(api, src, dst, util.DEFAULT_BUILD_PRODUCTS)
