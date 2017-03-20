# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import recipe_api

import default_flavor
import gn_android_flavor
import subprocess


"""GN Chromecast flavor utils, used for building Skia for Chromecast with GN"""
class GNChromecastFlavorUtils(gn_android_flavor.GNAndroidFlavorUtils):
  def __init__(self, m):
    super(GNChromecastFlavorUtils, self).__init__(m)

  def compile(self, unused_target):
    configuration = self.m.vars.builder_cfg.get('configuration')
    os            = self.m.vars.builder_cfg.get('os')
    target_arch   = self.m.vars.builder_cfg.get('target_arch')

    # Makes the binary small enough to fit on the small disk.
    extra_cflags = ['-g0']
    # Chromecast does not package libstdc++
    extra_ldflags = ['-static-libstdc++', '-static-libgcc']

    toolchain_dir = self.m.vars.slave_dir.join('cast_toolchain')

    quote = lambda x: '"%s"' % x
    args = {
      'cc': quote(toolchain_dir.join('bin','armv7a-cros-linux-gnueabi-gcc')),
      'cxx': quote(toolchain_dir.join('bin','armv7a-cros-linux-gnueabi-g++')),
      'ar': quote(toolchain_dir.join('bin','armv7a-cros-linux-gnueabi-ar')),
      'target_cpu': quote(target_arch),
      'skia_use_fontconfig': 'false',
      'skia_enable_gpu': 'false',
      # The toolchain won't allow system libraries to be used
      # when cross-compiling
      'skia_use_system_freetype2': 'false',
      # Makes the binary smaller
      'skia_use_icu': 'false',
    }

    if configuration != 'Debug':
      args['is_debug'] = 'false'
    args['extra_cflags'] = repr(extra_cflags).replace("'", '"')
    args['extra_ldflags'] = repr(extra_ldflags).replace("'", '"')

    gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.iteritems()))

    gn    = 'gn.exe'    if 'Win' in os else 'gn'
    ninja = 'ninja.exe' if 'Win' in os else 'ninja'
    gn = self.m.vars.skia_dir.join('bin', gn)

    self._py('fetch-gn', self.m.vars.skia_dir.join('bin', 'fetch-gn'))
    self._run('gn gen', gn, 'gen', self.out_dir, '--args=' + gn_args)
    # We only build perf for the chromecasts.
    self._run('ninja', ninja, '-C', self.out_dir, 'nanobench')

