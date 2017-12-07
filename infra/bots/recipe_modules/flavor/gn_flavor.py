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

    clang_linux        = str(self.m.vars.slave_dir.join('clang_linux'))
    emscripten_sdk     = str(self.m.vars.slave_dir.join('emscripten_sdk'))
    linux_vulkan_sdk   = str(self.m.vars.slave_dir.join('linux_vulkan_sdk'))
    win_toolchain = str(self.m.vars.slave_dir.join(
      't', 'depot_tools', 'win_toolchain', 'vs_files',
      'a9e1098bba66d2acccc377d5ee81265910f29272'))
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
    elif compiler == 'GCC' and os == "Ubuntu14":
      cc, cxx = 'gcc-4.8', 'g++-4.8'
    elif compiler == 'GCC':
      cc, cxx = 'gcc', 'g++'
    elif compiler == 'EMCC':
      cc   = emscripten_sdk + '/emscripten/incoming/emcc'
      cxx  = emscripten_sdk + '/emscripten/incoming/em++'
      extra_cflags.append('-Wno-unknown-warning-option')

    if 'Coverage' in extra_config:
      # See https://clang.llvm.org/docs/SourceBasedCodeCoverage.html for
      # more info on using llvm to gather coverage information.
      extra_cflags.append('-fprofile-instr-generate')
      extra_cflags.append('-fcoverage-mapping')
      extra_ldflags.append('-fprofile-instr-generate')
      extra_ldflags.append('-fcoverage-mapping')

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
    if extra_config == 'MSAN':
      args['skia_enable_gpu']     = 'false'
      args['skia_use_fontconfig'] = 'false'
    if 'ASAN' in extra_config or 'UBSAN' in extra_config:
      args['skia_enable_spirv_validation'] = 'false'
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
    if 'Vulkan' in extra_config and not 'Android' in extra_config:
      args['skia_enable_vulkan_debug_layers'] = 'false'
      if self.m.vars.is_linux:
        args['skia_vulkan_sdk'] = '"%s"' % linux_vulkan_sdk
      if 'Win' in os:
        args['skia_vulkan_sdk'] = '"%s"' % win_vulkan_sdk
    if 'Metal' in extra_config:
      args['skia_use_metal'] = 'true'
    if 'CheckGeneratedFiles' in extra_config:
      args['skia_compile_processors'] = 'true'
    if compiler == 'Clang' and 'Win' in os:
      args['clang_win'] = '"%s"' % self.m.vars.slave_dir.join('clang_win')
    if target_arch == 'wasm':
      args.update({
        'skia_use_freetype':   'false',
        'skia_use_fontconfig': 'false',
        'skia_use_dng_sdk':    'false',
        'skia_use_icu':        'false',
        'skia_enable_gpu':     'false',
      })

    sanitize = ''
    if 'SAN' in extra_config:
      sanitize = extra_config
    elif 'SafeStack' in extra_config:
      sanitize = 'safe-stack'

    for (k,v) in {
      'cc':  cc,
      'cxx': cxx,
      'sanitize': sanitize,
      'target_cpu': target_arch,
      'target_os': 'ios' if 'iOS' in extra_config else '',
      'win_sdk': win_toolchain + '/win_sdk' if 'Win' in os else '',
      'win_vc': win_toolchain + '/VC' if 'Win' in os else '',
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
      env = {}
      if 'CheckGeneratedFiles' in extra_config:
        env['PATH'] = '%s:%%(PATH)s' % self.m.vars.skia_dir.join('bin')
        self._py(
            'fetch-clang-format',
            self.m.vars.skia_dir.join('bin', 'fetch-clang-format'))
      if target_arch == 'wasm':
        fastcomp = emscripten_sdk + '/clang/fastcomp/build_incoming_64/bin'
        env['PATH'] = '%s:%%(PATH)s' % fastcomp

      with self.m.env(env):
        self._run('gn gen', [gn, 'gen', self.out_dir, '--args=' + gn_args])
        self._run('ninja', [ninja, '-k', '0', '-C', self.out_dir])

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
    path = []
    ld_library_path = []

    slave_dir = self.m.vars.slave_dir
    clang_linux = str(slave_dir.join('clang_linux'))
    extra_config = self.m.vars.builder_cfg.get('extra_config', '')

    if self.m.vars.is_linux:
      if (self.m.vars.builder_cfg.get('cpu_or_gpu', '') == 'GPU'
          and 'Intel' in self.m.vars.builder_cfg.get('cpu_or_gpu_value', '')):
        # The vulkan in this asset name simply means that the graphics driver
        # supports Vulkan. It is also the driver used for GL code.
        dri_path = slave_dir.join('linux_vulkan_intel_driver_release')
        if self.m.vars.builder_cfg.get('configuration', '') == 'Debug':
          dri_path = slave_dir.join('linux_vulkan_intel_driver_debug')
        ld_library_path.append(dri_path)
        env['LIBGL_DRIVERS_PATH'] = str(dri_path)
        env['VK_ICD_FILENAMES'] = str(dri_path.join('intel_icd.x86_64.json'))

      if 'Vulkan' in extra_config:
        path.append(slave_dir.join('linux_vulkan_sdk', 'bin'))
        ld_library_path.append(slave_dir.join('linux_vulkan_sdk', 'lib'))

    if 'SAN' in extra_config:
      # Sanitized binaries may want to run clang_linux/bin/llvm-symbolizer.
      path.append(clang_linux + '/bin')
    elif self.m.vars.is_linux:
      cmd = ['catchsegv'] + cmd

    if 'ASAN' == extra_config or 'UBSAN' in extra_config:
      env[ 'ASAN_OPTIONS'] = 'symbolize=1 detect_leaks=1'
      env[ 'LSAN_OPTIONS'] = 'symbolize=1 print_suppressions=1'
      env['UBSAN_OPTIONS'] = 'symbolize=1 print_stacktrace=1'

    if 'MSAN' == extra_config:
      # Find the MSAN-built libc++.
      ld_library_path.append(clang_linux + '/msan')

    if 'TSAN' == extra_config:
      # We don't care about malloc(), fprintf, etc. used in signal handlers.
      # If we're in a signal handler, we're already crashing...
      env['TSAN_OPTIONS'] = 'report_signal_unsafe=0'

    if 'Coverage' in extra_config:
      # This is the output file for the coverage data. Just running the binary
      # will produce the output. The output_file is in the swarming_out_dir and
      # thus will be an isolated output of the Test step.
      profname = '%s.profraw' % self.m.vars.builder_cfg.get('test_filter','o')
      env['LLVM_PROFILE_FILE'] = self.m.path.join(self.m.vars.swarming_out_dir,
                                                  profname)

    if path:
      env['PATH'] = '%%(PATH)s:%s' % ':'.join('%s' % p for p in path)
    if ld_library_path:
      env['LD_LIBRARY_PATH'] = ':'.join('%s' % p for p in ld_library_path)

    to_symbolize = ['dm', 'nanobench']
    if name in to_symbolize and self.m.vars.is_linux:
      # Convert path objects or placeholders into strings such that they can
      # be passed to symbolize_stack_trace.py
      args = [slave_dir] + [str(x) for x in cmd]
      with self.m.context(cwd=self.m.vars.skia_dir, env=env):
        self._py('symbolized %s' % name,
                 self.module.resource('symbolize_stack_trace.py'),
                 args=args,
                 infra_step=False)

    else:
      with self.m.context(env=env):
        self._run(name, cmd)
