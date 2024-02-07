# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DOCKER_IMAGE = 'gcr.io/skia-public/canvaskit-emsdk:3.1.26_v2'
INNER_BUILD_SCRIPT = '/SRC/skia/infra/canvaskit/build_canvaskit.sh'


def compile_fn(api, checkout_root, _ignore):
  skia_dir = checkout_root.join('skia')
  out_dir = api.vars.cache_dir.join('docker', 'canvaskit')
  configuration = api.vars.builder_cfg.get('configuration', '')
  extra = api.vars.builder_cfg.get('extra_config', '')

  # We want to make sure the directories exist and were created by chrome-bot,
  # because if that isn't the case, docker will make them and they will be
  # owned by root, which causes mysterious failures. To mitigate this risk
  # further, we don't use the same out_dir as everyone else (thus the _ignore)
  # param. Instead, we use a "canvaskit" subdirectory in the "docker" named_cache.
  api.file.ensure_directory('mkdirs out_dir', out_dir, mode=0o777)

  # Download the emsdk binaries (we won't actually use the ones on the Docker
  # image anymore, now that we have proper GN support)
  with api.context(cwd=skia_dir):
    api.run(api.step, 'activate-emsdk',
            cmd=['python3', skia_dir.join('bin', 'activate-emsdk')],
            infra_step=True)

  # This uses the emscripten sdk docker image and says "run the
  # build_canvaskit.sh helper script in there". Additionally, it binds two
  # folders: the Skia checkout to /SRC and the output directory to /OUT
  # The called helper script will make the compile happen and put the
  # output in the right spot.  The neat thing is that since the Skia checkout
  # (and, by extension, the build script) is not a part of the image, but
  # bound in at runtime, we don't have to re-build the image, except when the
  # toolchain changes.
  # Of note, the wasm build doesn't re-use any intermediate steps from the
  # previous builds, so it's essentially a build from scratch every time.
  cmd = ['docker', 'run', '--rm', '--volume', '%s:/SRC' % checkout_root,
         '--volume', '%s:/OUT' % out_dir,
         DOCKER_IMAGE, INNER_BUILD_SCRIPT]

  # The compile.sh script defaults to "gpu" which means Ganesh + WebGL. The other options are
  # "cpu" (no GPU backend) and "webgpu" (Graphite + WebGPU).
  if 'CPU' in extra:
    cmd.append('cpu')
  if 'WebGPU' in extra:
      cmd.append('webgpu')

  if configuration == 'Debug':
    cmd.append('debug') # It defaults to Release
  # Override DOCKER_CONFIG set by Kitchen.
  env = {'DOCKER_CONFIG': '/home/chrome-bot/.docker'}
  with api.env(env):
    api.run(
        api.step,
        'Build CanvasKit with Docker',
        cmd=cmd)


CANVASKIT_BUILD_PRODUCTS = [
  'canvaskit.*'
]


def copy_build_products(api, _ignore, dst):
  out_dir = api.vars.cache_dir.join('docker', 'canvaskit')
  # We don't use the normal copy_listed_files because it uses
  # shutil.move, which attempts to delete the previous file, which
  # doesn't work because the docker created outputs are read-only and
  # owned by root (aka only docker images). It's likely safe to change
  # the shutil.move in the original script to a non-deleting thing
  # (like copy or copyfile), but there's some subtle behavior differences
  # especially with directories, that kjlubick felt it best not to risk it.
  script = api.build.resource('copy_build_products_no_delete.py')
  api.step(
      name='copy wasm output',
      cmd=['python3', script, out_dir, dst, ','.join(CANVASKIT_BUILD_PRODUCTS)],
      infra_step=True)
