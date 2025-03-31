# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import default
from . import util

IMAGES = {
    'gcc-debian11': (
        'gcr.io/skia-public/gcc-debian11@sha256:'
        '1117ea368f43e45e0f543f74c8e3bf7ff6932df54ddaa4ba1fe6131209110d3d'),
    'gcc-debian11-x86': (
        'gcr.io/skia-public/gcc-debian11-x86@sha256:'
        'eb30682887c4c74c95f769aacab8a1a170eb561536ded87f0914f88b7243ba23'),
    'gcc-ubuntu18': (
        'gcr.io/skia-public/clang-ubuntu18@sha256:'
        'c46fdae57032646419baf125310c94363b5cb0e722ffcc63686468f485608aaa'),
}


def compile_fn(api, checkout_root, out_dir):
  compiler = api.vars.builder_cfg.get('compiler', '')
  extra_tokens = api.vars.extra_tokens
  extra_tokens.remove('Docker')
  os = api.vars.builder_cfg.get('os', '')
  target_arch = api.vars.builder_cfg.get('target_arch', '')

  workdir = api.path.cast_to_path('/SRC')
  args, env, _ = default.get_compile_flags(api, checkout_root, out_dir, workdir)

  image_name = None
  if os == 'Debian11' and compiler == 'GCC':
    if target_arch == 'x86_64':
      image_name = 'gcc-debian11'
    elif target_arch == 'x86':
      image_name = 'gcc-debian11-x86'
  elif os == 'Ubuntu18':
    # The Docker image contains GCC, but if we're building using Clang the CIPD
    # package will be provided as part of the task and mounted to the container.
    image_name = 'gcc-ubuntu18'

  if not image_name:
    raise Exception('Not implemented: ' + api.vars.builder_name)

  image_hash = IMAGES[image_name]

  args['extra_cflags'].append('-DREBUILD_IF_CHANGED_docker_image=%s' % image_hash)
  gn_flags = default.finalize_gn_flags(args)

  # We always perform an incremental compile, since out dir is cached across
  # compile tasks. However, we need to force a recompile when the toolchain
  # changes. The simplest way to do that is using a C define that changes
  # anytime the image changes.
  script = api.build.resource('docker-compile.sh')
  api.docker.run('Run build script in Docker', image_hash,
                 api.vars.workdir, out_dir, script, args=[gn_flags], env=env)

def copy_build_products(api, src, dst):
  util.copy_listed_files(api, src, dst, util.DEFAULT_BUILD_PRODUCTS)
