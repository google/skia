# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


"""Default flavor, used for running code on desktop machines."""


import collections


WIN_TOOLCHAIN_DIR = 't'


# Notes:
#   dm_dir: Where DM writes.
#   skp_dir: Holds SKP files that are consumed by RenderSKPs and BenchPictures.
DeviceDirs = collections.namedtuple(
    'DeviceDirs', ['bin_dir', 'dm_dir', 'perf_data_dir', 'resource_dir', 'images_dir', 'fonts_dir',
                   'lotties_dir', 'skp_dir', 'svg_dir', 'mskp_dir', 'tmp_dir', 'texttraces_dir'])


class DefaultFlavor(object):
  def __init__(self, module, app_name):
    # Name of the app we're going to run. May be used in various ways by
    # different flavors.
    self.app_name = app_name

    # Store a pointer to the parent recipe module (SkiaFlavorApi) so that
    # FlavorUtils objects can do recipe module-like things, like run steps or
    # access module-level resources.
    self.module = module

    # self.m is just a shortcut so that Flavor objects can use the same
    # syntax as regular recipe modules to run steps, eg: self.m.step(...)
    self.m = module.m
    self._chrome_path = None
    self.device_dirs = DeviceDirs(
        bin_dir=self.m.vars.build_dir,
        dm_dir=self.m.vars.swarming_out_dir,
        perf_data_dir=self.m.vars.swarming_out_dir,
        resource_dir=self.m.path['start_dir'].join('skia', 'resources'),
        images_dir=self.m.path['start_dir'].join('skimage'),
        fonts_dir=self.m.path['start_dir'].join('googlefonts_testdata', 'data'),
        lotties_dir=self.m.path['start_dir'].join('lottie-samples'),
        skp_dir=self.m.path['start_dir'].join('skp'),
        svg_dir=self.m.path['start_dir'].join('svg'),
        mskp_dir=self.m.path['start_dir'].join('mskp'),
        tmp_dir=self.m.vars.tmp_dir,
        texttraces_dir=self.m.path['start_dir'].join('text_blob_traces'))
    self.host_dirs = self.device_dirs

  def device_path_join(self, *args):
    """Like os.path.join(), but for paths on a connected device."""
    return self.m.path.join(*args)

  def copy_directory_contents_to_device(self, host_dir, device_dir):
    """Like shutil.copytree(), but for copying to a connected device."""
    # For "normal" builders who don't have an attached device, we expect
    # host_dir and device_dir to be the same.
    if str(host_dir) != str(device_dir):
      raise ValueError('For builders who do not have attached devices, copying '
                       'from host to device is undefined and only allowed if '
                       'host_path and device_path are the same (%s vs %s).' % (
                       str(host_dir), str(device_dir)))

  def copy_directory_contents_to_host(self, device_dir, host_dir):
    """Like shutil.copytree(), but for copying from a connected device."""
    # For "normal" builders who don't have an attached device, we expect
    # host_dir and device_dir to be the same.
    if str(host_dir) != str(device_dir):
      raise ValueError('For builders who do not have attached devices, copying '
                       'from device to host is undefined and only allowed if '
                       'host_path and device_path are the same (%s vs %s).' % (
                       str(host_dir), str(device_dir)))

  def copy_file_to_device(self, host_path, device_path):
    """Like shutil.copyfile, but for copying to a connected device."""
    # For "normal" builders who don't have an attached device, we expect
    # host_dir and device_dir to be the same.
    if str(host_path) != str(device_path):
      raise ValueError('For builders who do not have attached devices, copying '
                       'from host to device is undefined and only allowed if '
                       'host_path and device_path are the same (%s vs %s).' % (
                       str(host_path), str(device_path)))

  def create_clean_device_dir(self, path):
    """Like shutil.rmtree() + os.makedirs(), but on a connected device."""
    self.create_clean_host_dir(path)

  def create_clean_host_dir(self, path):
    """Convenience function for creating a clean directory."""
    self.m.run.rmtree(path)
    self.m.file.ensure_directory(
        'makedirs %s' % self.m.path.basename(path), path)

  def read_file_on_device(self, path, **kwargs):
    """Reads the specified file."""
    return self.m.file.read_text('read %s' % path, path)

  def remove_file_on_device(self, path):
    """Removes the specified file."""
    return self.m.file.remove('remove %s' % path, path)

  def install(self):
    """Run device-specific installation steps."""
    pass

  def cleanup_steps(self):
    """Run any device-specific cleanup steps."""
    pass

  def _run(self, title, cmd, infra_step=False, **kwargs):
    return self.m.run(self.m.step, title, cmd=cmd,
               infra_step=infra_step, **kwargs)

  def _py(self, title, script, infra_step=True, args=()):
    return self.m.run(self.m.step, title, cmd=['python3', script]+args,
                      infra_step=infra_step)

  def step(self, name, cmd, **unused_kwargs):
    app = self.device_dirs.bin_dir.join(cmd[0])
    cmd = [app] + cmd[1:]
    env = self.m.context.env
    path = []
    ld_library_path = []

    workdir = self.m.vars.workdir
    clang_linux = workdir.join('clang_linux')
    extra_tokens = self.m.vars.extra_tokens

    if self.m.vars.is_linux:
      if (self.m.vars.builder_cfg.get('cpu_or_gpu', '') == 'GPU'
          and 'Intel' in self.m.vars.builder_cfg.get('cpu_or_gpu_value', '')):
        dri_path = workdir.join('mesa_intel_driver_linux')
        if ('IntelIrisXe' in self.m.vars.builder_cfg.get('cpu_or_gpu_value', '')):
          dri_path = workdir.join('mesa_intel_driver_linux_22')
        ld_library_path.append(dri_path)
        env['LIBGL_DRIVERS_PATH'] = str(dri_path)
        env['VK_ICD_FILENAMES'] = str(dri_path.join('intel_icd.x86_64.json'))

      if 'Vulkan' in extra_tokens:
        env['VULKAN_SDK'] = str(workdir.join('linux_vulkan_sdk'))
        path.append(workdir.join('linux_vulkan_sdk', 'bin'))
        ld_library_path.append(workdir.join('linux_vulkan_sdk', 'lib'))
        # Enable layers for Debug only to avoid affecting perf results on
        # Release.
        # ASAN reports leaks in the Vulkan SDK when the debug layer is enabled.
        # TSAN runs out of memory.
        if (self.m.vars.builder_cfg.get('configuration', '') != 'Release' and
            'ASAN' not in extra_tokens and
            'TSAN' not in extra_tokens):
          env['VK_LAYER_PATH'] = str(workdir.join(
              'linux_vulkan_sdk', 'etc', 'vulkan', 'explicit_layer.d'))

    if 'SwiftShader' in extra_tokens:
      ld_library_path.append(self.host_dirs.bin_dir.join('swiftshader_out'))

    # Find the MSAN/TSAN-built libc++.
    if 'MSAN' in extra_tokens:
      ld_library_path.append(clang_linux.join('msan'))
    elif 'TSAN' in extra_tokens:
      ld_library_path.append(clang_linux.join('tsan'))

    if any('SAN' in t for t in extra_tokens):
      # Sanitized binaries may want to run clang_linux/bin/llvm-symbolizer.
      path.append(clang_linux.join('bin'))
      # We find that testing sanitizer builds with libc++ uncovers more issues
      # than with the system-provided C++ standard library, which is usually
      # libstdc++. libc++ proactively hooks into sanitizers to help their
      # analyses. We ship a copy of libc++ with our Linux toolchain in /lib.
      ld_library_path.append(clang_linux.join('lib', 'x86_64-unknown-linux-gnu'))

    if 'ASAN' in extra_tokens:
      os = self.m.vars.builder_cfg.get('os', '')
      if 'Mac' in os or 'Win' in os:
        # Mac and Win don't support detect_leaks.
        env['ASAN_OPTIONS'] = 'symbolize=1'
      else:
        env['ASAN_OPTIONS'] = 'symbolize=1 detect_leaks=1'
        env['ASAN_SYMBOLIZER_PATH'] = clang_linux.join('bin', 'llvm-symbolizer')
      env[ 'LSAN_OPTIONS'] = 'symbolize=1 print_suppressions=1'
      env['UBSAN_OPTIONS'] = 'symbolize=1 print_stacktrace=1'

      # If you see <unknown module> in stacktraces, try fast_unwind_on_malloc=0.
      # This may cause a 2-25x slowdown, so use it only when you really need it.
      if name == 'dm' and 'Vulkan' in extra_tokens:
        env['ASAN_OPTIONS'] += ' fast_unwind_on_malloc=0'
        env['LSAN_OPTIONS'] += ' fast_unwind_on_malloc=0'

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

    if 'DWriteCore' in extra_tokens:
      path.append(workdir.join('dwritecore', 'bin'))

    if path:
      env['PATH'] = self.m.path.pathsep.join(
          ['%(PATH)s'] + ['%s' % p for p in path])
    if ld_library_path:
      env['LD_LIBRARY_PATH'] = self.m.path.pathsep.join(
          '%s' % p for p in ld_library_path)

    to_symbolize = ['dm', 'nanobench']
    if name in to_symbolize and self.m.vars.is_linux:
      # Convert path objects or placeholders into strings such that they can
      # be passed to symbolize_stack_trace.py
      args = [workdir] + [str(x) for x in cmd]
      with self.m.context(cwd=self.m.path['start_dir'].join('skia'), env=env):
        self._py('symbolized %s' % name,
                 self.module.resource('symbolize_stack_trace.py'),
                 args=args,
                 infra_step=False)
    elif 'Win' in self.m.vars.builder_cfg.get('os', ''):
      with self.m.context(env=env):
        wrapped_cmd = ['powershell', '-ExecutionPolicy', 'Unrestricted',
                       '-File',
                       self.module.resource('win_run_and_check_log.ps1')] + cmd
        self._run(name, wrapped_cmd)
    else:
      with self.m.context(env=env):
        self._run(name, cmd)
