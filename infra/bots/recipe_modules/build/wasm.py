# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DOCKER_IMAGE = 'gcr.io/skia-public/emsdk-release:1.38.6'
BUILD_PRODUCTS_ISOLATE_WHITELIST_WASM = [
  'pathkit.*'
]

def compile_fn(api, checkout_root, out_dir):
  INNER_BUILD_SCRIPT = '/SRC/skia/experimental/pathkit/docker/build_pathkit.sh'

  api.run(
    api.step,
    'Build PathKit with Docker',
    cmd=['docker', 'run', '--rm', '-v', '%s:/SRC' % checkout_root,
         '-v', '%s:/OUT' % out_dir,
         DOCKER_IMAGE, INNER_BUILD_SCRIPT]
    )


def copy_extra_build_products(api, build_dir, dst):
  api.python.inline(
      name='copy more build products',
      program='''import errno
import glob
import os
import shutil
import sys

src = sys.argv[1]
dst = sys.argv[2]
build_products_whitelist = %s

try:
  os.makedirs(dst)
except OSError as e:
  if e.errno != errno.EEXIST:
    raise

for pattern in build_products_whitelist:
  path = os.path.join(src, pattern)
  for f in glob.glob(path):
    dst_path = os.path.join(dst, os.path.relpath(f, src))
    if not os.path.isdir(os.path.dirname(dst_path)):
      os.makedirs(os.path.dirname(dst_path))
    print 'Copying build product %%s to %%s' %% (f, dst_path)
    # Because Docker usually has some strange permissions (like root
    # ownership), we'd rather not keep those around.  copyfile doesn't
    # keep the metadata around, so that helps us.
    shutil.copyfile(f, dst_path)
''' % str(BUILD_PRODUCTS_ISOLATE_WHITELIST_WASM),
      args=[build_dir, dst],
      infra_step=True)

  api.run(
    api.step,
    'Cleanup docker-built files',
    cmd=['docker', 'run', '--rm', '-v', '%s:/OUT' % build_dir,
         DOCKER_IMAGE, 'rm /OUT/*']
    )
