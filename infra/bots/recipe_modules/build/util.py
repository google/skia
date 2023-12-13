# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Shared utilities for the build recipe module."""


# This lists the products we want to isolate as outputs for future steps.
DEFAULT_BUILD_PRODUCTS = [
  'dm',
  'dm.exe',
  'dm.app',
  'nanobench.app',
  'get_images_from_skps',
  'get_images_from_skps.exe',
  'nanobench',
  'nanobench.exe',
  'skpbench',
  'skpbench.exe',
  '*.so',
  '*.dll',
  '*.dylib',
  'skia_launcher',
  'skottie_tool',
  'lib/*.so',
  'run_testlab',
]

def py_to_gn(val):
  """Convert val to a string that can be used as GN args."""
  if isinstance(val, bool):
    return 'true' if val else 'false'
  elif '%s' % val == val:
    # TODO(dogben): Handle quoting "$\
    return '"%s"' % val
  elif isinstance(val, (list, tuple)):
    return '[%s]' % (','.join(py_to_gn(x) for x in val))
  elif isinstance(val, dict):
    gn = ' '.join(
        '%s=%s' % (k, py_to_gn(v)) for (k, v) in sorted(val.items()))
    return gn
  else:  # pragma: nocover
    raise Exception('Converting %s to gn is not implemented.' % type(val))


def copy_listed_files(api, src, dst, product_list):
  """Copy listed files src to dst."""
  api.python.inline(
      name='copy build products',
      program='''import errno
import glob
import os
import shutil
import sys

src = sys.argv[1]
dst = sys.argv[2]
build_products = %s

try:
  os.makedirs(dst)
except OSError as e:
  if e.errno != errno.EEXIST:
    raise

for pattern in build_products:
  path = os.path.join(src, pattern)
  for f in glob.glob(path):
    dst_path = os.path.join(dst, os.path.relpath(f, src))
    if not os.path.isdir(os.path.dirname(dst_path)):
      os.makedirs(os.path.dirname(dst_path))
    print('Copying build product %%s to %%s' %% (f, dst_path))
    shutil.move(f, dst_path)
''' % str(product_list),
      args=[src, dst],
      infra_step=True)


def set_dawn_args_and_env(args, env, api, extra_tokens, skia_dir):
    """Add to ``args`` and ``env`` the gn args and environment vars needed to
    make a build targeting Dawn."""
    args['skia_use_dawn'] = 'true'
    args['skia_use_gl'] = 'false'
    # Set dawn specific args to limit which backends are built
    args['dawn_enable_d3d11'] = 'false'
    args['dawn_enable_d3d12'] = 'false'
    args['dawn_enable_metal'] = 'false'
    args['dawn_enable_desktop_gl'] = 'false'
    args['dawn_enable_opengles'] = 'false'
    args['dawn_enable_vulkan'] = 'false'
    if 'D3D11' in extra_tokens:
      args['dawn_enable_d3d11'] = 'true'
    if 'D3D12' in extra_tokens:
      args['dawn_enable_d3d12'] = 'true'
    if 'Metal' in extra_tokens:
      args['dawn_enable_metal'] = 'true'
    if 'Vulkan' in extra_tokens:
      args['dawn_enable_vulkan'] = 'true'
    env['PYTHONPATH'] = api.path.pathsep.join([
        str(skia_dir.join('third_party', 'externals')), '%%(PYTHONPATH)s'])
