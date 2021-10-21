# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import re

from . import util

def compile_fn(api, checkout_root, out_dir):
  skia_dir      = checkout_root.join('skia')
  compiler      = api.vars.builder_cfg.get('compiler')
  configuration = api.vars.builder_cfg.get('configuration')
  extra_tokens  = api.vars.extra_tokens
  os            = api.vars.builder_cfg.get('os')
  target_arch   = api.vars.builder_cfg.get('target_arch')

  assert compiler == 'Clang'  # At this rate we might not ever support GCC.

  extra_cflags = []
  if configuration == 'Debug':
    extra_cflags.append('-O1')

  ndk_asset = 'android_ndk_linux'
  ndk_path = ndk_asset
  if 'Mac' in os:
    ndk_asset = 'android_ndk_darwin'
    ndk_path = ndk_asset
  elif 'Win' in os:
    ndk_asset = 'android_ndk_windows'
    ndk_path = 'n'

  quote = lambda x: '"%s"' % x
  args = {
      'ndk': quote(api.vars.workdir.join(ndk_path)),
      'target_cpu': quote(target_arch),
      'werror': 'true',
  }
  extra_cflags.append('-DREBUILD_IF_CHANGED_ndk_version=%s' %
                      api.run.asset_version(ndk_asset, skia_dir))

  if configuration != 'Debug':
    args['is_debug'] = 'false'
  if 'Vulkan' in extra_tokens:
    args['ndk_api'] = 24
    args['skia_enable_vulkan_debug_layers'] = 'false'
    args['skia_use_gl'] = 'false'
  if 'ASAN' in extra_tokens:
    args['sanitize'] = '"ASAN"'
  if 'HWASAN' in extra_tokens:
    args['sanitize'] = '"HWASAN"'
  if 'Wuffs' in extra_tokens:
    args['skia_use_wuffs'] = 'true'

  # If an Android API level is specified, use that.
  for t in extra_tokens:
    m = re.search(r'API(\d+)', t)
    if m and len(m.groups()) == 1:
      args['ndk_api'] = m.groups()[0]
      break

  if extra_cflags:
    args['extra_cflags'] = repr(extra_cflags).replace("'", '"')

  gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.iteritems()))
  gn      = skia_dir.join('bin', 'gn')

  with api.context(cwd=skia_dir):
    api.run(api.python, 'fetch-gn',
            script=skia_dir.join('bin', 'fetch-gn'),
            infra_step=True)

    api.run(api.step, 'gn gen',
            cmd=[gn, 'gen', out_dir, '--args=' + gn_args])
    api.run(api.step, 'ninja', cmd=['ninja', '-C', out_dir])


ANDROID_BUILD_PRODUCTS_LIST = [
  'dm',
  'nanobench',
  'skpbench',
]


def copy_build_products(api, src, dst):
  """Copy Android build products from src to dst."""
  util.copy_listed_files(api, src, dst, ANDROID_BUILD_PRODUCTS_LIST)
