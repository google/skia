# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DOCKER_IMAGE = 'gcr.io/skia-public/cmake-release:3.13.1_v2'
INNER_BUILD_SCRIPT = '/SRC/skia/infra/cmake/build_skia.sh'


def compile_fn(api, checkout_root, _ignore):
  out_dir = api.vars.cache_dir.join('docker', 'cmake')
  configuration = api.vars.builder_cfg.get('configuration', '')
  if configuration != 'Release': # pragma: nocover
    # If a debug mode is wanted, update the infra/cmake/build_skia.sh
    # to support that also.
    raise 'Only Release mode supported for CMake'

  # We want to make sure the directories exist and were created by chrome-bot,
  # because if that isn't the case, docker will make them and they will be
  # owned by root, which causes mysterious failures. To mitigate this risk
  # further, we don't use the same out_dir as everyone else (thus the _ignore)
  # param. Instead, we use a "cmake" subdirectory in the "docker" named_cache.
  api.file.ensure_directory('mkdirs out_dir', out_dir, mode=0777)

  # This uses the cmake docker image and says "run the
  # build_skia.sh helper script in there". Additionally, it binds two
  # folders: the Skia checkout to /SRC and the output directory to /OUT
  # The called helper script will make the compile happen and put the
  # output in the right spot.  The neat thing is that since the Skia checkout
  # (and, by extension, the build script) is not a part of the image, but
  # bound in at runtime, we don't have to re-build the image, except when the
  # toolchain changes.
  cmd = ['docker', 'run', '--rm', '--volume', '%s:/SRC' % checkout_root,
         '--volume', '%s:/OUT' % out_dir,
         DOCKER_IMAGE, INNER_BUILD_SCRIPT]
  # It's always Release mode (for now?) This is mostly an FYI
  # thing, to make sure we don't break CMake users.

  # Override DOCKER_CONFIG set by Kitchen.
  env = {'DOCKER_CONFIG': '/home/chrome-bot/.docker'}
  with api.env(env):
    api.run(
        api.step,
        'Build Skia using CMake in Docker',
        cmd=cmd)

def copy_extra_build_products(_api, _src, _dst):
  pass
