# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


"""Default flavor, used for running code on desktop machines."""


WIN_TOOLCHAIN_DIR = 't'


class DeviceDirs(object):
  def __init__(self,
               bin_dir,
               dm_dir,
               perf_data_dir,
               resource_dir,
               images_dir,
               lotties_dir,
               skp_dir,
               svg_dir,
               tmp_dir):
    self._bin_dir = bin_dir
    self._dm_dir = dm_dir
    self._perf_data_dir = perf_data_dir
    self._resource_dir = resource_dir
    self._images_dir = images_dir
    self._lotties_dir = lotties_dir
    self._skp_dir = skp_dir
    self._svg_dir = svg_dir
    self._tmp_dir = tmp_dir

  @property
  def bin_dir(self):
    return self._bin_dir

  @property
  def dm_dir(self):
    """Where DM writes."""
    return self._dm_dir

  @property
  def perf_data_dir(self):
    return self._perf_data_dir

  @property
  def resource_dir(self):
    return self._resource_dir

  @property
  def images_dir(self):
    return self._images_dir

  @property
  def lotties_dir(self):
    return self._lotties_dir

  @property
  def skp_dir(self):
    """Holds SKP files that are consumed by RenderSKPs and BenchPictures."""
    return self._skp_dir

  @property
  def svg_dir(self):
    return self._svg_dir

  @property
  def tmp_dir(self):
    return self._tmp_dir


class DefaultFlavor(object):
  def __init__(self, module):
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
        lotties_dir=self.m.path['start_dir'].join('lottie-samples'),
        skp_dir=self.m.path['start_dir'].join('skp'),
        svg_dir=self.m.path['start_dir'].join('svg'),
        tmp_dir=self.m.vars.tmp_dir)
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
    return self.m.run(self.m.python, title, script=script, args=args,
               infra_step=infra_step)

  def step(self, name, cmd):
    app = self.device_dirs.bin_dir.join(cmd[0])
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
        dri_path = slave_dir.join('mesa_intel_driver_linux')
        ld_library_path.append(dri_path)
        env['LIBGL_DRIVERS_PATH'] = str(dri_path)
        env['VK_ICD_FILENAMES'] = str(dri_path.join('intel_icd.x86_64.json'))

      if 'Vulkan' in extra_tokens:
        path.append(slave_dir.join('linux_vulkan_sdk', 'bin'))
        ld_library_path.append(slave_dir.join('linux_vulkan_sdk', 'lib'))

      if 'OpenCL' in extra_tokens:
        ld_library_path.append(slave_dir.join('opencl_ocl_icd_linux'))
        # TODO(dogben): Limit to the appropriate GPUs when we start running on
        # GPUs other than IntelIris640.
        # Skylake and later use the NEO driver.
        neo_path = slave_dir.join('opencl_intel_neo_linux')
        ld_library_path.append(neo_path)
        # Generate vendors dir contaning the ICD file pointing to the NEO OpenCL
        # library.
        vendors_dir = self.m.vars.tmp_dir.join('OpenCL', 'vendors')
        self.m.file.ensure_directory('mkdirs OpenCL/vendors', vendors_dir)
        self.m.file.write_raw('write NEO OpenCL ICD',
                              vendors_dir.join('neo.icd'),
                              '%s\n' % neo_path.join('libigdrcl.so'))
        env['OPENCL_VENDOR_PATH'] = vendors_dir

    if 'SwiftShader' in extra_tokens:
      ld_library_path.append(self.host_dirs.bin_dir.join('swiftshader_out'))

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
      dumps_dir = self.m.path.join(self.m.vars.swarming_out_dir, 'dumps')
      self.m.file.ensure_directory('makedirs dumps', dumps_dir)
      procdump = str(self.m.vars.slave_dir.join('procdump_win',
                                                'procdump64.exe'))
      # Full docs for ProcDump here:
      # https://docs.microsoft.com/en-us/sysinternals/downloads/procdump
      # -accepteula automatically accepts the license agreement
      # -mp saves a packed minidump to save space
      # -e 1 tells procdump to dump once
      # -x <dump dir> <exe> <args> launches exe and writes dumps to the
      #   specified dir
      cmd = [procdump, '-accepteula', '-mp', '-e', '1', '-x', dumps_dir] + cmd

    if 'ASAN' in extra_tokens or 'UBSAN' in extra_tokens:
      # Note: if you see "<unknown module>" in stacktraces for xSAN warnings,
      # try adding "fast_unwind_on_malloc=0" to xSAN_OPTIONS.
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
      with self.m.context(cwd=self.m.path['start_dir'].join('skia'), env=env):
        self._py('symbolized %s' % name,
                 self.module.resource('symbolize_stack_trace.py'),
                 args=args,
                 infra_step=False)

    else:
      with self.m.context(env=env):
        self._run(name, cmd)
