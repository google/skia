# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import recipe_api

import gn_flavor
import subprocess


"""
  GN Chromebook flavor utils, used for building and testing Skia for ARM
  Chromebooks with GN
"""
class GNChromebookFlavorUtils(gn_flavor.GNFlavorUtils):

  def compile(self, unused_target):
    configuration = self.m.vars.builder_cfg.get('configuration')
    os            = self.m.vars.builder_cfg.get('os')
    target_arch   = self.m.vars.builder_cfg.get('target_arch')
    extra_config  = self.m.vars.builder_cfg.get('extra_config', '')

    clang_linux = self.m.vars.slave_dir.join('clang_linux')
    # This is a pretty typical arm-linux-gnueabihf sysroot
    sysroot_dir = self.m.vars.slave_dir.join('armhf_sysroot')
    # This is the extra things needed to link against for the chromebook.
    #  For example, the Mali GL drivers.
    if extra_config == 'Chromebook_C100p':
      gl_dir   = self.m.vars.slave_dir.join('chromebook_c100p_lib')

    extra_asmflags = [
      '--target=armv7a-linux-gnueabihf',
      '--sysroot=%s' % sysroot_dir,
      '-march=armv7-a',
      '-mfpu=neon',
      '-mthumb',
    ]

    extra_cflags = [
      '--target=armv7a-linux-gnueabihf',
      '--sysroot=%s' % sysroot_dir,
      '-I%s' % gl_dir.join('include'),
      '-I%s' % sysroot_dir.join('include'),
      '-I%s' % sysroot_dir.join('include', 'c++', '4.8.4'),
      '-I%s' % sysroot_dir.join('include', 'c++', '4.8.4',
                                'arm-linux-gnueabihf'),
       '-DMESA_EGL_NO_X11_HEADERS',
    ]

    extra_ldflags = [
      '--target=armv7a-linux-gnueabihf',
      '--sysroot=%s' % sysroot_dir,
      # use sysroot's ld which can properly link things.
      '-B%s' % sysroot_dir.join('bin'),
      # helps locate crt*.o
      '-B%s' % sysroot_dir.join('gcc-cross'),
      # helps locate libgcc*.so
      '-L%s' % sysroot_dir.join('gcc-cross'),
      '-L%s' % sysroot_dir.join('lib'),
      '-L%s' % gl_dir.join('lib'),
      # Explicitly do not use lld for cross compiling like this - I observed
      # failures like "Unrecognized reloc 41" and couldn't find out why.
    ]

    quote = lambda x: '"%s"' % x
    args = {
      'cc': quote(clang_linux.join('bin','clang')),
      'cxx': quote(clang_linux.join('bin','clang++')),
      'target_cpu': quote(target_arch),
      'skia_use_fontconfig': 'false',
      'skia_use_system_freetype2': 'false',
      'skia_use_egl': 'true',
    }

    if configuration != 'Debug':
      args['is_debug'] = 'false'
    args['extra_asmflags'] = repr(extra_asmflags).replace("'", '"')
    args['extra_cflags'] = repr(extra_cflags).replace("'", '"')
    args['extra_ldflags'] = repr(extra_ldflags).replace("'", '"')

    gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.iteritems()))

    gn    = 'gn.exe'    if 'Win' in os else 'gn'
    ninja = 'ninja.exe' if 'Win' in os else 'ninja'
    gn = self.m.vars.skia_dir.join('bin', gn)

    context = {
      'cwd': self.m.vars.skia_dir,
      'env': {
          'LD_LIBRARY_PATH': sysroot_dir.join('lib'),
      }
    }
    with self.m.step.context(context):
      self._py('fetch-gn', self.m.vars.skia_dir.join('bin', 'fetch-gn'))
      self._run('gn gen', [gn, 'gen', self.out_dir, '--args=' + gn_args])
      self._run('ninja', [ninja, '-C', self.out_dir, 'nanobench', 'dm'])
