# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Shared utilities for the build recipe module."""


BUILD_PRODUCTS_ISOLATE_WHITELIST = [
  'dm',
  'dm.exe',
  'dm.app',
  'nanobench.app',
  'get_images_from_skps',
  'get_images_from_skps.exe',
  'hello-opencl',
  'hello-opencl.exe',
  'nanobench',
  'nanobench.exe',
  'skpbench',
  'skpbench.exe',
  '*.so',
  '*.dll',
  '*.dylib',
  'skia_launcher',
  'skiaserve',
  'skottie_tool',
  'lib/*.so',
  'run_testlab',
  'skqp-universal-debug.apk',
  'whitelist_devices.json',
]


def copy_whitelisted_build_products(api, src, dst):
  """Copy whitelisted build products from src to dst."""
  api.python.inline(
      name='copy build products',
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
    shutil.move(f, dst_path)
''' % str(BUILD_PRODUCTS_ISOLATE_WHITELIST),
      args=[src, dst],
      infra_step=True)
