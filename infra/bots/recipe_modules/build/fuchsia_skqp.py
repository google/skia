# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DOCKER_IMAGE = 'gcr.io/skia-public/android-skqp:r20_v1'
INNER_BUILD_DIR = '/SRC/skia/infra/skqp'
INNER_BUILD_SCRIPT = './build_apk.sh'

def compile_fn(api, checkout_root, out_dir):
    api.python.inline(
      name='sleep for debugging',
      program='''import time

for i in range(30):
  print("sleeping and hacking")
  time.sleep(60)

''')
  # api.docker.run('Run build script in Docker', image_hash,
  #                checkout_root, out_dir, script, args=[py_to_gn(args)])


SKQP_BUILD_PRODUCTS = [
  'TODO',
]


def copy_build_products(api, src, dst):
  pass
  # util.copy_listed_files(api, src, dst, SKQP_BUILD_PRODUCTS)
