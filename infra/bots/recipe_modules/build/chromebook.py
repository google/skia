# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from . import util
import os

def compile_fn(api, checkout_root, out_dir):
  skia_dir      = checkout_root.join('skia')
  configuration = api.vars.builder_cfg.get('configuration')
  target_arch   = api.vars.builder_cfg.get('target_arch')

  top_level = str(api.vars.workdir)

  clang_linux = os.path.join(top_level, 'clang_linux')
  # This is a pretty typical arm-linux-gnueabihf sysroot
  sysroot_dir = os.path.join(top_level, 'armhf_sysroot')

  args = {
    'cc': "%s" % os.path.join(clang_linux, 'bin','clang'),
    'cxx': "%s" % os.path.join(clang_linux, 'bin','clang++'),
    'extra_cflags' : [],
    'extra_ldflags' : [],
    'extra_asmflags' : [],
    'target_cpu': target_arch,
    'skia_use_fontconfig': False,
    'skia_use_system_freetype2': False,
    'skia_use_egl': True,
    'werror': True,
  }

  if 'arm' == target_arch:
    # This is the extra things needed to link against for the chromebook.
    #  For example, the Mali GL drivers.
    gl_dir = os.path.join(top_level, 'chromebook_arm_gles')
    env = {'LD_LIBRARY_PATH': os.path.join(sysroot_dir, 'lib')}
    args['extra_asmflags'] = [
      '--target=armv7a-linux-gnueabihf',
      '--sysroot=%s' % sysroot_dir,
      '-march=armv7-a',
      '-mfpu=neon',
      '-mthumb',
    ]

    args['extra_cflags'] = [
      '--target=armv7a-linux-gnueabihf',
      '--sysroot=%s' % sysroot_dir,
      '-I%s' % os.path.join(gl_dir, 'include'),
      '-I%s' % os.path.join(sysroot_dir, 'include'),
      '-I%s' % os.path.join(sysroot_dir, 'include', 'c++', '10'),
      '-I%s' % os.path.join(sysroot_dir, 'include', 'c++', '10', 'arm-linux-gnueabihf'),
      '-DMESA_EGL_NO_X11_HEADERS',
      '-U_GLIBCXX_DEBUG',
    ]

    args['extra_ldflags'] = [
      '--target=armv7a-linux-gnueabihf',
      '--sysroot=%s' % sysroot_dir,
      '-static-libstdc++', '-static-libgcc',
      # use sysroot's ld which can properly link things.
      '-B%s' % os.path.join(sysroot_dir, 'bin'),
      # helps locate crt*.o
      '-B%s' % os.path.join(sysroot_dir, 'gcc-cross'),
      # helps locate libgcc*.so
      '-L%s' % os.path.join(sysroot_dir, 'gcc-cross'),
      '-L%s' % os.path.join(sysroot_dir, 'lib'),
      '-L%s' % os.path.join(gl_dir, 'lib'),
    ]
  else:
    gl_dir = os.path.join(top_level,'chromebook_x86_64_gles')
    env = {}
    args['extra_asmflags'] = []
    args['extra_cflags'] = [
      '-DMESA_EGL_NO_X11_HEADERS',
      '-I%s' % os.path.join(gl_dir, 'include'),
    ]
    args['extra_ldflags'] = [
      '-L%s' % os.path.join(gl_dir, 'lib'),
      '-static-libstdc++', '-static-libgcc',
      '-fuse-ld=lld',
    ]

  args['extra_cflags'].append('-DREBUILD_IF_CHANGED_clang_linux_version=%s' %
                      api.run.asset_version('clang_linux', skia_dir))

  if configuration != 'Debug':
    args['is_debug'] = False

  gn = skia_dir.join('bin', 'gn')

  with api.context(cwd=skia_dir, env=env):
    api.run(api.step, 'fetch-gn',
            cmd=['python3', skia_dir.join('bin', 'fetch-gn')],
            infra_step=True)
    api.run(api.step, 'gn gen',
            cmd=[gn, 'gen', out_dir, '--args=' + util.py_to_gn(args)])
    api.run(api.step, 'ninja',
            cmd=['ninja', '-C', out_dir, 'nanobench', 'dm'])


def copy_build_products(api, src, dst):
  util.copy_listed_files(api, src, dst, util.DEFAULT_BUILD_PRODUCTS)
