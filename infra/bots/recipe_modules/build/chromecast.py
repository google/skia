# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


def compile_fn(api, checkout_root, out_dir):
  skia_dir      = checkout_root.join('skia')
  configuration = api.vars.builder_cfg.get('configuration')
  target_arch   = api.vars.builder_cfg.get('target_arch')

  # TODO(kjlubick): can this toolchain be replaced/shared with chromebook?
  toolchain_dir = api.vars.slave_dir.join('cast_toolchain', 'armv7a')
  gles_dir = api.vars.slave_dir.join('chromebook_arm_gles')

  target  = ['-target', 'armv7a-cros-linux-gnueabi']
  sysroot = ['--sysroot',
             '%s/usr/armv7a-cros-linux-gnueabi' % toolchain_dir]

  extra_asmflags = target

  extra_cflags = target + sysroot + [
    '-I%s' % gles_dir.join('include'),
    '-DMESA_EGL_NO_X11_HEADERS',
    "-DSK_NO_COMMAND_BUFFER",
    # Avoid unused warning with yyunput
    '-Wno-error=unused-function',
    # Makes the binary small enough to fit on the small disk.
    '-g0',
    ('-DDUMMY_cast_toolchain_version=%s' %
     api.run.asset_version('cast_toolchain', skia_dir)),
  ]

  extra_ldflags = target + sysroot + [
    # Chromecast does not package libstdc++
    '-static-libstdc++', '-static-libgcc',
    '-L%s' % toolchain_dir.join('lib'),
    '-fuse-ld=gold',
    '-B%s/usr/libexec/gcc' % toolchain_dir,
  ]

  quote = lambda x: '"%s"' % x
  args = {
    'cc':  quote(toolchain_dir.join('usr', 'bin', 'clang-3.9.elf')),
    'cxx': quote(toolchain_dir.join('usr', 'bin', 'clang++-3.9.elf')),
    'ar':  quote(toolchain_dir.join('bin','armv7a-cros-linux-gnueabi-ar')),
    'target_cpu': quote(target_arch),
    'skia_use_fontconfig': 'false',
    'skia_enable_gpu': 'true',
    # The toolchain won't allow system libraries to be used
    # when cross-compiling
    'skia_use_system_freetype2': 'false',
    # Makes the binary smaller
    'skia_use_icu': 'false',
    'skia_use_egl': 'true',
    'werror': 'true',
  }

  if configuration != 'Debug':
    args['is_debug'] = 'false'
  args['extra_asmflags'] = repr(extra_asmflags).replace("'", '"')
  args['extra_cflags'] = repr(extra_cflags).replace("'", '"')
  args['extra_ldflags'] = repr(extra_ldflags).replace("'", '"')

  gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.iteritems()))
  gn = skia_dir.join('bin', 'gn')

  with api.context(cwd=skia_dir):
    api.run(api.python, 'fetch-gn',
            script=skia_dir.join('bin', 'fetch-gn'),
            infra_step=True)
    api.run(api.step, 'gn gen', cmd=[gn, 'gen', out_dir, '--args=' + gn_args])
    api.run(api.step, 'ninja',
            cmd=['ninja', '-C', out_dir, 'nanobench', 'dm'])


def copy_extra_build_products(api, src, dst):
  pass
