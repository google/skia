# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import default_flavor

"""GN flavor utils, used for building Skia with GN."""
class GNFlavorUtils(default_flavor.DefaultFlavorUtils):
  def _run(self, title, cmd, infra_step=False, **kwargs):
    return self.m.run(self.m.step, title, cmd=cmd,
               infra_step=infra_step, **kwargs)

  def _py(self, title, script, infra_step=True, args=()):
    return self.m.run(self.m.python, title, script=script, args=args,
               infra_step=infra_step)

  def build_command_buffer(self):
    self.m.run(self.m.python, 'build command_buffer',
        script=self.m.vars.skia_dir.join('tools', 'build_command_buffer.py'),
        args=[
          '--chrome-dir', self.m.vars.checkout_root,
          '--output-dir', self.m.vars.skia_out.join(self.m.vars.configuration),
          '--no-sync', '--make-output-dir'])

  def compile(self, unused_target):
    """Build Skia with GN."""
    compiler      = self.m.vars.builder_cfg.get('compiler',      '')
    configuration = self.m.vars.builder_cfg.get('configuration', '')
    extra_config  = self.m.vars.builder_cfg.get('extra_config',  '')
    os            = self.m.vars.builder_cfg.get('os',            '')
    target_arch   = self.m.vars.builder_cfg.get('target_arch',   '')

    clang_linux   = str(self.m.vars.slave_dir.join('clang_linux'))
    linux_vulkan_sdk   = str(self.m.vars.slave_dir.join('linux_vulkan_sdk'))
    win_toolchain = str(self.m.vars.slave_dir.join(
      't', 'depot_tools', 'win_toolchain', 'vs_files',
      'd3cb0e37bdd120ad0ac4650b674b09e81be45616'))
    win_vulkan_sdk = str(self.m.vars.slave_dir.join('win_vulkan_sdk'))

    cc, cxx = None, None
    extra_cflags = []
    extra_ldflags = []

    if compiler == 'Clang' and self.m.vars.is_linux:
      cc  = clang_linux + '/bin/clang'
      cxx = clang_linux + '/bin/clang++'
      extra_cflags .append('-B%s/bin' % clang_linux)
      extra_ldflags.append('-B%s/bin' % clang_linux)
      extra_ldflags.append('-fuse-ld=lld')
    elif compiler == 'Clang':
      cc, cxx = 'clang', 'clang++'
    elif compiler == 'GCC':
      cc, cxx = 'gcc', 'g++'

    if compiler != 'MSVC' and configuration == 'Debug':
      extra_cflags.append('-O1')

    if extra_config == 'Exceptions':
      extra_cflags.append('/EHsc')
    if extra_config == 'Fast':
      extra_cflags.extend(['-march=native', '-fomit-frame-pointer', '-O3',
                           '-ffp-contract=off'])
    if extra_config.startswith('SK'):
      extra_cflags.append('-D' + extra_config)
    if extra_config == 'MSAN':
      extra_ldflags.append('-L' + clang_linux + '/msan')

    args = {}

    if configuration != 'Debug':
      args['is_debug'] = 'false'
    if extra_config == 'ANGLE':
      args['skia_use_angle'] = 'true'
    if extra_config == 'CommandBuffer':
      self.m.run.run_once(self.build_command_buffer)
    if extra_config == 'GDI':
      args['skia_use_gdi'] = 'true'
    if extra_config == 'MSAN':
      args['skia_enable_gpu']     = 'false'
      args['skia_use_fontconfig'] = 'false'
    if extra_config == 'ASAN':
      args['skia_enable_spirv_validation'] = 'false'
    if extra_config == 'Mesa':
      args['skia_use_mesa'] = 'true'
    if extra_config == 'Mini':
      args.update({
        'is_component_build':     'true',   # Proves we can link a coherent .so.
        'is_official_build':      'true',   # No debug symbols, no tools.
        'skia_enable_effects':    'false',
        'skia_enable_gpu':        'true',
        'skia_enable_pdf':        'false',
        'skia_use_expat':         'false',
        'skia_use_libjpeg_turbo': 'false',
        'skia_use_libpng':        'false',
        'skia_use_libwebp':       'false',
        'skia_use_zlib':          'false',
      })
    if extra_config == 'NoGPU':
      args['skia_enable_gpu'] = 'false'
    if extra_config == 'Shared':
      args['is_component_build'] = 'true'
    if extra_config == 'Vulkan':
      args['skia_enable_vulkan_debug_layers'] = 'false'
      if self.m.vars.is_linux:
        args['skia_vulkan_sdk'] = '"%s"' % linux_vulkan_sdk
      if 'Win' in os:
        args['skia_vulkan_sdk'] = '"%s"' % win_vulkan_sdk

    for (k,v) in {
      'cc':  cc,
      'cxx': cxx,
      'sanitize': extra_config if 'SAN' in extra_config else '',
      'target_cpu': target_arch,
      'target_os': 'ios' if 'iOS' in extra_config else '',
      'windk': win_toolchain if 'Win' in os else '',
    }.iteritems():
      if v:
        args[k] = '"%s"' % v
    if extra_cflags:
      args['extra_cflags'] = repr(extra_cflags).replace("'", '"')
    if extra_ldflags:
      args['extra_ldflags'] = repr(extra_ldflags).replace("'", '"')

    gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.iteritems()))

    gn    = 'gn.exe'    if 'Win' in os else 'gn'
    ninja = 'ninja.exe' if 'Win' in os else 'ninja'
    gn = self.m.vars.skia_dir.join('bin', gn)

    with self.m.context(cwd=self.m.vars.skia_dir):
      self._py('fetch-gn', self.m.vars.skia_dir.join('bin', 'fetch-gn'))
      self._run('gn gen', [gn, 'gen', self.out_dir, '--args=' + gn_args])
      self._run('ninja', [ninja, '-C', self.out_dir])

  def copy_extra_build_products(self, swarming_out_dir):
    configuration = self.m.vars.builder_cfg.get('configuration', '')
    extra_config  = self.m.vars.builder_cfg.get('extra_config',  '')
    os            = self.m.vars.builder_cfg.get('os',            '')

    win_vulkan_sdk = str(self.m.vars.slave_dir.join('win_vulkan_sdk'))
    if 'Win' in os and extra_config == 'Vulkan':
      self.m.run.copy_build_products(
          win_vulkan_sdk,
          swarming_out_dir.join('out', configuration + '_x64'))

  def step(self, name, cmd):
    app = self.m.vars.skia_out.join(self.m.vars.configuration, cmd[0])
    cmd = [app] + cmd[1:]
    env = self.m.context.env

    clang_linux = str(self.m.vars.slave_dir.join('clang_linux'))
    extra_config = self.m.vars.builder_cfg.get('extra_config', '')

    if 'SAN' in extra_config:
      # Sanitized binaries may want to run clang_linux/bin/llvm-symbolizer.
      env['PATH'] = '%%(PATH)s:%s' % clang_linux + '/bin'
    elif self.m.vars.is_linux:
      cmd = ['catchsegv'] + cmd

    if 'ASAN' == extra_config:
      env[ 'ASAN_OPTIONS'] = 'symbolize=1 detect_leaks=1'
      env[ 'LSAN_OPTIONS'] = 'symbolize=1 print_suppressions=1'
      env['UBSAN_OPTIONS'] = 'symbolize=1 print_stacktrace=1'

    if 'MSAN' == extra_config:
      # Find the MSAN-built libc++.
      env['LD_LIBRARY_PATH'] = clang_linux + '/msan'

    to_symbolize = ['dm', 'nanobench']
    if name in to_symbolize and self.m.vars.is_linux:
      # Convert path objects or placeholders into strings such that they can
      # be passed to symbolize_stack_trace.py
      args = [self.m.vars.slave_dir] + [str(x) for x in cmd]
      with self.m.context(cwd=self.m.vars.skia_dir, env=env):
        self._py('symbolized %s' % name,
                 self.module.resource('symbolize_stack_trace.py'),
                 args=args,
                 infra_step=False)

    else:
      with self.m.context(env=env):
        self._run(name, cmd)
