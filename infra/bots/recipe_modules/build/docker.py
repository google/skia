# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import util

IMAGES = {
    'gcc-debian10': (
        'gcr.io/skia-public/gcc-debian10@sha256:'
        'cd1dd99a3c423332a00998a20c4363aa1d5998b41f21e6e86ca016b412082777'),
    'gcc-debian10-x86': (
        'gcr.io/skia-public/gcc-debian10-x86@sha256:'
        'e30b4616f842fa2fd89329abf3d8e81cf6c25e147640289f37692f18862515c8'),
}


def compile_fn(api, checkout_root, out_dir):
  compiler = api.vars.builder_cfg.get('compiler', '')
  configuration = api.vars.builder_cfg.get('configuration', '')
  extra_tokens = api.vars.extra_tokens
  extra_tokens.remove('Docker')
  os = api.vars.builder_cfg.get('os', '')
  target_arch = api.vars.builder_cfg.get('target_arch', '')

  args = {
      'extra_cflags': [],
      'extra_ldflags': [],
      'target_cpu': target_arch,
      'werror': True
  }

  if configuration == 'Debug':
    args['extra_cflags'].append('-O1')
  else:
    args['is_debug'] = False

  if 'NoGPU' in extra_tokens:
    args['skia_enable_ganesh'] = False
    extra_tokens.remove('NoGPU')
  if 'Shared' in extra_tokens:
    args['is_component_build'] = True
    extra_tokens.remove('Shared')

  image_name = None
  if os == 'Debian10' and compiler == 'GCC' and not extra_tokens:
    args['cc'] = 'gcc'
    args['cxx'] = 'g++'
    # Newer GCC includes tons and tons of debugging symbols. This seems to
    # negatively affect our bots (potentially only in combination with other
    # bugs in Swarming or recipe code). Use g1 to reduce it a bit.
    args['extra_cflags'].append('-g1')
    if target_arch == 'x86_64':
      image_name = 'gcc-debian10'
    elif target_arch == 'x86':
      image_name = 'gcc-debian10-x86'

  if not image_name:
    raise Exception('Not implemented: ' + api.vars.builder_name)

  image_hash = IMAGES[image_name]
  # We always perform an incremental compile, since out dir is cached across
  # compile tasks. However, we need to force a recompile when the toolchain
  # changes. The simplest way to do that is using a C define that changes
  # anytime the image changes.
  args['extra_cflags'].append('-DREBUILD_IF_CHANGED_docker_image=%s' % image_hash)

  script = api.build.resource('docker-compile.sh')
  api.docker.run('Run build script in Docker', image_hash,
                 checkout_root, out_dir, script, args=[util.py_to_gn(args)])

def copy_build_products(api, src, dst):
  util.copy_listed_files(api, src, dst, util.DEFAULT_BUILD_PRODUCTS)
