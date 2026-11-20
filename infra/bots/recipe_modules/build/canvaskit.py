# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from . import util


def compile_fn(api, checkout_root, out_dir):
  skia_dir = checkout_root.joinpath('skia')
  configuration = api.vars.builder_cfg.get('configuration', '')
  extra = api.vars.builder_cfg.get('extra_config', '')

  with api.context(cwd=skia_dir):
    api.run(api.step, 'activate-emsdk',
            cmd=['python3', skia_dir.joinpath('bin', 'activate-emsdk')],
            infra_step=True)

    cmd = [skia_dir.joinpath('modules', 'canvaskit', 'compile.sh')]

    # The compile.sh script defaults to "gpu" which means Ganesh + WebGL. The
    # other options are "cpu" (no GPU backend) and "webgpu" (Graphite + WebGPU).
    if 'CPU' in extra:
      cmd.append('cpu')
    if 'WebGPU' in extra:
      cmd.append('webgpu')

    if configuration == 'Debug':
      cmd.append('debug')  # It defaults to Release

    with api.env({'BUILD_DIR': str(out_dir)}):
      api.run(
          api.step,
          'Build CanvasKit',
          cmd=cmd)


CANVASKIT_BUILD_PRODUCTS = [
  'canvaskit.*'
]


def copy_build_products(api, src, dst):
  util.copy_listed_files(api, src, dst, CANVASKIT_BUILD_PRODUCTS)
