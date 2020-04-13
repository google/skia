# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from . import util

IMAGES = {
    # Used to build ChromeOS for Pixelbook in Debian9, to align GLIBC versions.
    'debian9': (
        'gcr.io/skia-public/debian9:latest'),
        #'gcr.io/skia-public/debian9@sha256:'
        #'7d2666afcdf7e64e0e1830980e9b051d58c6f2ac74a22a6187c319412bccb736'),
}

def py_to_gn(val):
  """Convert val to a string that can be used as GN args."""
  if isinstance(val, bool):
    return 'true' if val else 'false'
  elif isinstance(val, basestring):
    # TODO(dogben): Handle quoting "$\
    return '"%s"' % val
  elif isinstance(val, (list, tuple)):
    return '[%s]' % (','.join(py_to_gn(x) for x in val))
  elif isinstance(val, dict):
    gn = ' '.join(
        '%s=%s' % (k, py_to_gn(v)) for (k, v) in sorted(val.iteritems()))
    return gn
  else:  # pragma: nocover
    raise Exception('Converting %s to gn is not implemented.' % type(val))

def compile_fn(api, checkout_root, out_dir):
  skia_dir      = checkout_root.join('skia')
  configuration = api.vars.builder_cfg.get('configuration')
  target_arch   = api.vars.builder_cfg.get('target_arch')
  os = api.vars.builder_cfg.get('os', '')
  builder_name = api.vars.builder_name

  clang_linux = api.vars.slave_dir.join('clang_linux')
  # This is a pretty typical arm-linux-gnueabihf sysroot
  sysroot_dir = api.vars.slave_dir.join('armhf_sysroot')

  args = {
    'cc': quote(clang_linux.join('bin','clang')),
    'cxx': quote(clang_linux.join('bin','clang++')),
    'extra_cflags' : [],
    'extra_ldflags' : [],
    'extra_asmflags' : [],
    'target_cpu': quote(target_arch),
    'skia_use_fontconfig': 'false',
    'skia_use_system_freetype2': 'false',
    'skia_use_egl': 'true',
    'werror': 'true',
  }

  if 'arm' == target_arch:
    # This is the extra things needed to link against for the chromebook.
    #  For example, the Mali GL drivers.
    gl_dir = api.vars.slave_dir.join('chromebook_arm_gles')
    env = {'LD_LIBRARY_PATH': sysroot_dir.join('lib')}
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
      '-I%s' % gl_dir.join('include'),
      '-I%s' % sysroot_dir.join('include'),
      '-I%s' % sysroot_dir.join('include', 'c++', '6'),
      '-I%s' % sysroot_dir.join('include', 'c++', '6', 'arm-linux-gnueabihf'),
      '-DMESA_EGL_NO_X11_HEADERS',
      '-U_GLIBCXX_DEBUG',
    ]

    args['extra_ldflags'] = [
      '--target=armv7a-linux-gnueabihf',
      '--sysroot=%s' % sysroot_dir,
      '-static-libstdc++', '-static-libgcc',
      # use sysroot's ld which can properly link things.
      '-B%s' % sysroot_dir.join('bin'),
      # helps locate crt*.o
      '-B%s' % sysroot_dir.join('gcc-cross'),
      # helps locate libgcc*.so
      '-L%s' % sysroot_dir.join('gcc-cross'),
      '-L%s' % sysroot_dir.join('lib'),
      '-L%s' % gl_dir.join('lib'),
    ]
  else:
    gl_dir = api.vars.slave_dir.join('chromebook_x86_64_gles')
    env = {}
    args['extra_asmflags'] = []
    args['extra_cflags'] = [
      '-DMESA_EGL_NO_X11_HEADERS',
      '-I%s' % gl_dir.join('include'),
    ]
    args['extra_ldflags'] = [
      '-L%s' % gl_dir.join('lib'),
      '-static-libstdc++', '-static-libgcc',
      '-fuse-ld=lld',
    ]

  args['extra_cflags'].append('-DDUMMY_clang_linux_version=%s' %
                      api.run.asset_version('clang_linux', skia_dir))

  if configuration != 'Debug':
    args['is_debug'] = 'false'
  #args['extra_asmflags'] = repr(extra_asmflags).replace("'", '"')
  #args['extra_cflags'] = repr(extra_cflags).replace("'", '"')
  #args['extra_ldflags'] = repr(extra_ldflags).replace("'", '"')

  gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.iteritems()))
  gn = skia_dir.join('bin', 'gn')

  if True or os == 'Debian9' and 'Docker' in builder_name:
    script = api.build.resource('docker-chromeos-compile.sh')
    image_name = 'debian9'
    image_hash = IMAGES[image_name]
    api.docker.run('Run build script in Docker', image_hash,
                   checkout_root, out_dir, script, args=[py_to_gn(args)])
    return

  with api.context(cwd=skia_dir, env=env):
    api.run(api.python, 'fetch-gn',
            script=skia_dir.join('bin', 'fetch-gn'),
            infra_step=True)
    api.run(api.step, 'gn gen', cmd=[gn, 'gen', out_dir, '--args=' + gn_args])
    api.run(api.step, 'ninja',
            cmd=['ninja', '-C', out_dir, 'nanobench', 'dm'])


def copy_build_products(api, src, dst):
  util.copy_listed_files(api, src, dst, util.DEFAULT_BUILD_PRODUCTS)
