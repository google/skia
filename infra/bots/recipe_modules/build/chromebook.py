# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from . import util

def compile_fn(api, checkout_root, out_dir):
  skia_dir      = checkout_root.join('skia')
  configuration = api.vars.builder_cfg.get('configuration')
  target_arch   = api.vars.builder_cfg.get('target_arch')

  clang_linux = api.vars.slave_dir.join('clang_linux')
  # This is a pretty typical arm-linux-gnueabihf sysroot
  sysroot_dir = api.vars.slave_dir.join('armhf_sysroot')

  if 'arm' == target_arch:
    # This is the extra things needed to link against for the chromebook.
    #  For example, the Mali GL drivers.
    gl_dir = api.vars.slave_dir.join('chromebook_arm_gles')
    env = {'LD_LIBRARY_PATH': sysroot_dir.join('lib')}
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
      '-U_GLIBCXX_DEBUG',
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
  else:
    gl_dir = api.vars.slave_dir.join('chromebook_x86_64_gles')
    env = {}
    extra_asmflags = []
    extra_cflags = [
      '-DMESA_EGL_NO_X11_HEADERS',
      '-I%s' % gl_dir.join('include'),
    ]
    extra_ldflags = [
      '-L%s' % gl_dir.join('lib'),
      '-static-libstdc++', '-static-libgcc',
      '-fuse-ld=lld',
    ]

  quote = lambda x: '"%s"' % x
  args = {
    'cc': quote(clang_linux.join('bin','clang')),
    'cxx': quote(clang_linux.join('bin','clang++')),
    'target_cpu': quote(target_arch),
    'skia_use_fontconfig': 'false',
    'skia_use_system_freetype2': 'false',
    'skia_use_egl': 'true',
    'werror': 'true',
  }
  extra_cflags.append('-DDUMMY_clang_linux_version=%s' %
                      api.run.asset_version('clang_linux', skia_dir))

  if configuration != 'Debug':
    args['is_debug'] = 'false'
  args['extra_asmflags'] = repr(extra_asmflags).replace("'", '"')
  args['extra_cflags'] = repr(extra_cflags).replace("'", '"')
  args['extra_ldflags'] = repr(extra_ldflags).replace("'", '"')

  gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.iteritems()))
  gn = skia_dir.join('bin', 'gn')

  with api.context(cwd=skia_dir, env=env):
    api.run(api.python, 'fetch-gn',
            script=skia_dir.join('bin', 'fetch-gn'),
            infra_step=True)
    api.run(api.step, 'gn gen', cmd=[gn, 'gen', out_dir, '--args=' + gn_args])
    api.run(api.step, 'ninja',
            cmd=['ninja', '-C', out_dir, 'nanobench', 'dm'])


def copy_build_products(api, src, dst):
  util.copy_listed_files(api, src, dst, util.DEFAULT_BUILD_PRODUCTS)
