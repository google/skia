# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import default_flavor

"""GN flavor utils, used for building Skia with GN."""
class GNFlavorUtils(default_flavor.DefaultFlavorUtils):
  def compile(self, target):
    """Build Skia with GN."""
    # Get the gn executable.
    fetch_gn = self._skia_api.skia_dir.join('bin', 'fetch-gn')
    self._skia_api.run(self._skia_api.m.step, 'fetch-gn', cmd=[fetch_gn],
                       cwd=self._skia_api.skia_dir)

    is_debug = 'is_debug=true'
    if self._skia_api.configuration != 'Debug':
        is_debug = 'is_debug=false'
    gn_args = [is_debug]

    is_clang = 'Clang' in self._skia_api.builder_name
    is_gcc   = 'GCC'   in self._skia_api.builder_name

    cc, cxx = 'cc', 'c++'
    if is_clang:
      cc, cxx = 'clang', 'clang++'
    elif is_gcc:
      cc, cxx = 'gcc', 'g++'

    ccache = self._skia_api.ccache()
    if ccache:
      cc, cxx = '%s %s' % (ccache, cc), '%s %s' % (ccache, cxx)
      if is_clang:
        # Stifle "argument unused during compilation: ..." warnings.
        stifle = '-Qunused-arguments'
        cc, cxx = '%s %s' % (cc, stifle), '%s %s' % (cxx, stifle)

    gn_args += [ 'cc="%s"' % cc, 'cxx="%s"' % cxx ]

    # Run gn gen.
    gn_exe = 'gn'
    if self._skia_api.m.platform.is_win:
        gn_exe = 'gn.exe'
    gn_gen = [gn_exe, 'gen', self.out_dir, '--args=%s' % ' '.join(gn_args)]
    self._skia_api.run(self._skia_api.m.step, 'gn_gen', cmd=gn_gen,
                       cwd=self._skia_api.skia_dir)

    # Run ninja.
    ninja_cmd = ['ninja', '-C', self.out_dir]
    self._skia_api.run(self._skia_api.m.step, 'compile %s' % target,
            cmd=ninja_cmd,
            cwd=self._skia_api.skia_dir)
