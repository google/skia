# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import default_flavor

"""GN flavor utils, used for building Skia with GN."""
class GNFlavorUtils(default_flavor.DefaultFlavorUtils):
  def supported(self):
    extra_config = self.m.vars.builder_cfg.get('extra_config', '')

    return any([
      extra_config == 'GN',
      extra_config == 'Fast',
      extra_config.startswith('SK')
    ])

  def compile(self, unused_target, **kwargs):
    """Build Skia with GN."""
    compiler      = self.m.vars.builder_cfg.get('compiler',      '')
    configuration = self.m.vars.builder_cfg.get('configuration', '')
    extra_config  = self.m.vars.builder_cfg.get('extra_config',  '')

    gn_args = []
    if configuration != 'Debug':
      gn_args.append('is_debug=false')

    cc, cxx = 'cc', 'c++'
    cflags = []

    if compiler == 'Clang':
      cc, cxx = 'clang', 'clang++'
    elif compiler == 'GCC':
      cc, cxx = 'gcc', 'g++'

    ccache = self.m.run.ccache()
    if ccache:
      cc, cxx = '%s %s' % (ccache, cc), '%s %s' % (ccache, cxx)
      if compiler == 'Clang':
        # Stifle "argument unused during compilation: ..." warnings.
        cflags.append('-Qunused-arguments')

    if extra_config == 'Fast':
      cflags.extend(['-march=native', '-fomit-frame-pointer'])
    if extra_config.startswith('SK'):
      cflags.append('-D' + extra_config)

    cflags = ' '.join(cflags)
    gn_args += [ 'cc="%s %s"' % (cc, cflags), 'cxx="%s %s"' % (cxx, cflags) ]

    run = lambda title, cmd: self.m.run(self.m.step, title, cmd=cmd,
                                        cwd=self.m.vars.skia_dir, **kwargs)

    run('fetch-gn', [self.m.vars.skia_dir.join('bin', 'fetch-gn')])
    run('gn gen', ['gn', 'gen', self.out_dir, '--args=%s' % ' '.join(gn_args)])
    run('ninja', ['ninja', '-C', self.out_dir])
