# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import default_flavor

"""GN Android flavor utils, used for building Skia for Android with GN."""
class GNAndroidFlavorUtils(default_flavor.DefaultFlavorUtils):
  def supported(self):
    return 'GN_Android' == self.m.vars.builder_cfg.get('extra_config', '')

  def compile(self, unused_target, **kwargs):
    compiler      = self.m.vars.builder_cfg.get('compiler')
    configuration = self.m.vars.builder_cfg.get('configuration')
    os            = self.m.vars.builder_cfg.get('os')
    target_arch   = self.m.vars.builder_cfg.get('target_arch')

    assert compiler == 'Clang'  # At this rate we might not ever support GCC.

    compiler_prefix = ''
    extra_cflags = []
    ccache = self.m.run.ccache()
    if ccache:
      compiler_prefix = ccache
      extra_cflags.append('-Qunused-arguments')

    ndk_asset = 'android_ndk_linux' if os == 'Ubuntu' else 'android_ndk_darwin'

    quote = lambda x: '"%s"' % x
    gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted({
        'compiler_prefix': quote(compiler_prefix),
        'extra_cflags': quote(' '.join(extra_cflags)),
        'is_debug': 'true' if configuration == 'Debug' else 'false',
        'ndk': quote(self.m.vars.slave_dir.join(ndk_asset)),
        'target_cpu': quote(target_arch),
    }.iteritems()))

    run = lambda title, cmd: self.m.run(self.m.step, title, cmd=cmd,
                                        cwd=self.m.vars.skia_dir, **kwargs)

    run('fetch-gn', [self.m.vars.skia_dir.join('bin', 'fetch-gn')])
    run('gn gen', ['gn', 'gen', self.out_dir, '--args=' + gn_args])
    run('ninja', ['ninja', '-C', self.out_dir])
