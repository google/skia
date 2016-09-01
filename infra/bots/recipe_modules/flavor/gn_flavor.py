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

  def _run(self, title, cmd):
    path = self.m.vars.default_env['PATH']
    self.m.vars.default_env = {'PATH': path}
    self.m.run(self.m.step, title, cmd=cmd, cwd=self.m.vars.skia_dir)

  def compile(self, unused_target, **kwargs):
    """Build Skia with GN."""
    compiler      = self.m.vars.builder_cfg.get('compiler',      '')
    configuration = self.m.vars.builder_cfg.get('configuration', '')
    extra_config  = self.m.vars.builder_cfg.get('extra_config',  '')
    os            = self.m.vars.builder_cfg.get('os',            '')

    cc, cxx = 'cc', 'c++'
    extra_cflags = []

    if compiler == 'Clang' and os == 'Ubuntu':
      cc  = self.m.vars.slave_dir.join('clang_linux', 'bin', 'clang')
      cxx = self.m.vars.slave_dir.join('clang_linux', 'bin', 'clang++')
    elif compiler == 'Clang':
      cc, cxx = 'clang', 'clang++'
    elif compiler == 'GCC':
      cc, cxx = 'gcc', 'g++'

    compiler_prefix = ""
    ccache = self.m.run.ccache()
    if ccache:
      compiler_prefix = ccache
      if compiler == 'Clang':
        # Stifle "argument unused during compilation: ..." warnings.
        extra_cflags.append('-Qunused-arguments')

    if extra_config == 'Fast':
      extra_cflags.extend(['-march=native', '-fomit-frame-pointer', '-O3'])
    if extra_config.startswith('SK'):
      extra_cflags.append('-D' + extra_config)

    quote = lambda x: '"%s"' % x
    gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted({
        'cc': quote(cc),
        'cxx': quote(cxx),
        'compiler_prefix': quote(compiler_prefix),
        'extra_cflags': quote(' '.join(extra_cflags)),
        'is_debug': 'true' if configuration == 'Debug' else 'false',
    }.iteritems()))

    self._run('fetch-gn', [self.m.vars.skia_dir.join('bin', 'fetch-gn')])
    self._run('gn gen', ['gn', 'gen', self.out_dir, '--args=' + gn_args])
    self._run('ninja', ['ninja', '-C', self.out_dir])
