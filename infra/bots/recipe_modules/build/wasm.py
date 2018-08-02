# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


def compile_fn(api, checkout_root, out_dir):
  DOCKER_IMAGE = 'gcr.io/skia-public/emsdk-release:1.38.6'
  INNER_BUILD_SCRIPT = '/SRC/experimental/pathkit/docker/build_pathkit.sh'
  api.run(
    api.step,
    'Build PathKit with Docker',
    cmd=['docker', 'run', '--rm', '-v', '%s:/SRC' % checkout_root,
         '-v', '%s:/OUT' % out_dir,
         DOCKER_IMAGE, INNER_BUILD_SCRIPT]
    )

def copy_extra_build_products(api, out_dir, dst):
  pass
