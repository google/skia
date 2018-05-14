# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import default_flavor

"""GN flavor utils, used for building Skia with GN."""
class GNFlavorUtils(default_flavor.DefaultFlavorUtils):
  # TODO(borenet): Delete this file.
  def _run(self, title, cmd, infra_step=False, **kwargs):
    return self.m.run(self.m.step, title, cmd=cmd,
               infra_step=infra_step, **kwargs)

  def _py(self, title, script, infra_step=True, args=()):
    return self.m.run(self.m.python, title, script=script, args=args,
               infra_step=infra_step)

  def step(self, name, cmd):
    app = self.m.vars.skia_out.join(self.m.vars.configuration, cmd[0])
    cmd = [app] + cmd[1:]
    env = self.m.context.env
    path = []
    ld_library_path = []

    slave_dir = self.m.vars.slave_dir
    clang_linux = str(slave_dir.join('clang_linux'))
    extra_tokens = self.m.vars.extra_tokens

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

      if 'Vulkan' in extra_tokens:
        path.append(slave_dir.join('linux_vulkan_sdk', 'bin'))
        ld_library_path.append(slave_dir.join('linux_vulkan_sdk', 'lib'))

    if 'SwiftShader' in extra_tokens:
      ld_library_path.append(self.m.vars.skia_out.join('swiftshader_out'))

    if 'MSAN' in extra_tokens:
      # Find the MSAN-built libc++.
      ld_library_path.append(clang_linux + '/msan')

    if any('SAN' in t for t in extra_tokens):
      # Sanitized binaries may want to run clang_linux/bin/llvm-symbolizer.
      path.append(clang_linux + '/bin')
      # We find that testing sanitizer builds with libc++ uncovers more issues
      # than with the system-provided C++ standard library, which is usually
      # libstdc++. libc++ proactively hooks into sanitizers to help their
      # analyses. We ship a copy of libc++ with our Linux toolchain in /lib.
      ld_library_path.append(clang_linux + '/lib')
    elif self.m.vars.is_linux:
      cmd = ['catchsegv'] + cmd
    elif 'ProcDump' in extra_tokens:
      self.m.file.ensure_directory('makedirs dumps', self.m.vars.dumps_dir)
      procdump = str(self.m.vars.slave_dir.join('procdump_win',
                                                'procdump64.exe'))
      # Full docs for ProcDump here:
      # https://docs.microsoft.com/en-us/sysinternals/downloads/procdump
      # -accepteula automatically accepts the license agreement
      # -mp saves a packed minidump to save space
      # -e 1 tells procdump to dump once
      # -x <dump dir> <exe> <args> launches exe and writes dumps to the
      #   specified dir
      cmd = [procdump, '-accepteula', '-mp', '-e', '1',
             '-x', self.m.vars.dumps_dir] + cmd

    if 'ASAN' in extra_tokens or 'UBSAN' in extra_tokens:
      if 'Mac' in self.m.vars.builder_cfg.get('os', ''):
        env['ASAN_OPTIONS'] = 'symbolize=1'  # Mac doesn't support detect_leaks.
      else:
        env['ASAN_OPTIONS'] = 'symbolize=1 detect_leaks=1'
      env[ 'LSAN_OPTIONS'] = 'symbolize=1 print_suppressions=1'
      env['UBSAN_OPTIONS'] = 'symbolize=1 print_stacktrace=1'

    if 'TSAN' in extra_tokens:
      # We don't care about malloc(), fprintf, etc. used in signal handlers.
      # If we're in a signal handler, we're already crashing...
      env['TSAN_OPTIONS'] = 'report_signal_unsafe=0'

    if 'Coverage' in extra_tokens:
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
