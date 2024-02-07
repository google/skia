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
  extra_ldflags = []
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
  env = {}
  extra_cflags.append('-DREBUILD_IF_CHANGED_ndk_version=%s' %
                      api.run.asset_version(ndk_asset, skia_dir))

  if configuration != 'Debug':
    args['is_debug'] = 'false'
  if 'Dawn' in extra_tokens:
    util.set_dawn_args_and_env(args, env, api, extra_tokens, skia_dir)
    args['ndk_api'] = 26 #skia_use_gl=false, so use vulkan
  if 'Vulkan' in extra_tokens and not 'Dawn' in extra_tokens:
    args['ndk_api'] = 26
    args['skia_enable_vulkan_debug_layers'] = 'false'
    args['skia_use_gl'] = 'false'
    args['skia_use_vulkan'] = 'true'
  if 'ASAN' in extra_tokens:
    args['sanitize'] = '"ASAN"'
  if 'Graphite' in extra_tokens:
    args['skia_enable_graphite'] = 'true'
  if 'HWASAN' in extra_tokens:
    args['sanitize'] = '"HWASAN"'
  if 'Wuffs' in extra_tokens:
    args['skia_use_wuffs'] = 'true'
  if configuration == 'OptimizeForSize':
    # build IDs are required for Bloaty if we want to use strip to ignore debug symbols.
    # https://github.com/google/bloaty/blob/master/doc/using.md#debugging-stripped-binaries
    extra_ldflags.append('-Wl,--build-id=sha1')
    args.update({
      'skia_use_runtime_icu': 'true',
      'skia_enable_optimize_size': 'true',
      'skia_use_jpeg_gainmaps': 'false',
    })

  # The 'FrameworkWorkarounds' bot is used to test special behavior that's
  # normally enabled with SK_BUILD_FOR_ANDROID_FRAMEWORK.
  if 'FrameworkWorkarounds' in extra_tokens:
    extra_cflags.append('-DSK_SUPPORT_LEGACY_ALPHA_BITMAP_AS_COVERAGE')

  # If an Android API level is specified, use that.
  for t in extra_tokens:
    m = re.search(r'API(\d+)', t)
    if m and len(m.groups()) == 1:
      args['ndk_api'] = m.groups()[0]
      break

  if extra_cflags:
    args['extra_cflags'] = repr(extra_cflags).replace("'", '"')
  if extra_ldflags:
    args['extra_ldflags'] = repr(extra_ldflags).replace("'", '"')

  gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.items()))
  gn      = skia_dir.join('bin', 'gn')

  with api.context(cwd=skia_dir):
    api.run(api.step, 'fetch-gn',
            cmd=['python3', skia_dir.join('bin', 'fetch-gn')],
            infra_step=True)

    with api.env(env):
      api.run(api.step, 'gn gen',
              cmd=[gn, 'gen', out_dir, '--args=' + gn_args])
      api.run(api.step, 'ninja', cmd=['ninja', '-C', out_dir])


ANDROID_BUILD_PRODUCTS_LIST = [
  'dm',
  'nanobench',
  'skpbench',
  # The following only exists when building for OptimizeForSize
  # This is the only target we currently measure: skbug.com/13657
  'skottie_tool_gpu',
]


def copy_build_products(api, src, dst):
  """Copy Android build products from src to dst."""
  util.copy_listed_files(api, src, dst, ANDROID_BUILD_PRODUCTS_LIST)
