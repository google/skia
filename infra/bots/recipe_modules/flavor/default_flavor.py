# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


"""Default flavor utils class, used for desktop builders."""


import json


WIN_TOOLCHAIN_DIR = 't'


class DeviceDirs(object):
  def __init__(self,
               dm_dir,
               perf_data_dir,
               resource_dir,
               images_dir,
               skp_dir,
               svg_dir,
               tmp_dir):
    self._dm_dir = dm_dir
    self._perf_data_dir = perf_data_dir
    self._resource_dir = resource_dir
    self._images_dir = images_dir
    self._skp_dir = skp_dir
    self._svg_dir = svg_dir
    self._tmp_dir = tmp_dir

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
  def skp_dir(self):
    """Holds SKP files that are consumed by RenderSKPs and BenchPictures."""
    return self._skp_dir

  @property
  def svg_dir(self):
    return self._svg_dir

  @property
  def tmp_dir(self):
    return self._tmp_dir


class DefaultFlavorUtils(object):
  """Utilities to be used by build steps.

  The methods in this class define how certain high-level functions should
  work. Each build step flavor should correspond to a subclass of
  DefaultFlavorUtils which may override any of these functions as appropriate
  for that flavor.

  For example, the AndroidFlavorUtils will override the functions for
  copying files between the host and Android device, as well as the
  'step' function, so that commands may be run through ADB.
  """
  def __init__(self, m):
    self.m = m
    self._chrome_path = None
    self._win_toolchain_dir = self.m.vars.slave_dir.join(WIN_TOOLCHAIN_DIR)
    win_toolchain_asset_path = self.m.vars.infrabots_dir.join(
        'assets', 'win_toolchain', 'VERSION')
    if not self.m.path.exists(win_toolchain_asset_path):
      self._win_toolchain_dir = self.m.vars.slave_dir


  def step(self, name, cmd, **kwargs):
    """Wrapper for the Step API; runs a step as appropriate for this flavor."""
    path_to_app = self.m.vars.skia_out.join(
        self.m.vars.configuration, cmd[0])
    if (self.m.platform.is_linux and
        'x86_64' in self.m.vars.builder_name and
        not 'TSAN' in self.m.vars.builder_name):
      new_cmd = ['catchsegv', path_to_app]
    else:
      new_cmd = [path_to_app]
    new_cmd.extend(cmd[1:])
    return self.m.run(self.m.step,
                              name, cmd=new_cmd, **kwargs)

  @property
  def chrome_path(self):
    """Path to a checkout of Chrome on this machine."""
    return self._win_toolchain_dir.join('src')

  def bootstrap_win_toolchain(self):
    """Run bootstrapping script for the Windows toolchain."""
    bootstrap_script = self.m.vars.infrabots_dir.join(
        'bootstrap_win_toolchain_json.py')
    win_toolchain_json = self._win_toolchain_dir.join(
        'src', 'build', 'win_toolchain.json')
    self.m.python(
        'bootstrap win toolchain',
        script=bootstrap_script,
        args=['--win_toolchain_json', win_toolchain_json,
              '--depot_tools_parent_dir',
              self._win_toolchain_dir])

  def compile(self, target, **kwargs):
    """Build the given target."""
    env = kwargs.pop('env', {})
    # The CHROME_PATH environment variable is needed for builders that use
    # toolchains downloaded by Chrome.
    env['CHROME_PATH'] = self.chrome_path
    if self.m.platform.is_win:
      make_cmd = ['python', 'make.py']
      self.m.run.run_once(self.bootstrap_win_toolchain)
      if 'Vulkan' in self.m.vars.builder_name:
        env['VULKAN_SDK'] = self.m.vars.slave_dir.join('win_vulkan_sdk')
    else:
      make_cmd = ['make']
    cmd = make_cmd + [target]
    try:
      self.m.run(self.m.step, 'build %s' % target, cmd=cmd,
                 env=env, cwd=self.m.path['checkout'], **kwargs)
    except self.m.step.StepFailure:
      if self.m.platform.is_win:
        # The linker occasionally crashes on Windows. Try again.
        self.m.run(self.m.step, 'build %s' % target, cmd=cmd,
                   env=env, cwd=self.m.path['checkout'], **kwargs)
      else:
        raise

  def copy_extra_build_products(self, swarming_out_dir):
    """Copy extra build products to specified directory.

    Copy flavor-specific build products to swarming_out_dir for use in test and
    perf steps."""
    if ("Win" in self.m.vars.builder_name and
        "Vulkan" in self.m.vars.builder_name):
      # This copies vulkan-1.dll that has been bundled into win_vulkan_sdk
      # since version 2  See skia/api BUILD_PRODUCTS_ISOLATE_WHITELIST
      self.m.run.copy_build_products(
          self.m.path['slave_build'].join('win_vulkan_sdk'),
          swarming_out_dir)

  @property
  def out_dir(self):
    """Flavor-specific out directory."""
    return self.m.vars.skia_out.join(self.m.vars.configuration)

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
                       str(host_dir), str(device_dir)))  # pragma: no cover

  def copy_directory_contents_to_host(self, device_dir, host_dir):
    """Like shutil.copytree(), but for copying from a connected device."""
    # For "normal" builders who don't have an attached device, we expect
    # host_dir and device_dir to be the same.
    if str(host_dir) != str(device_dir):
      raise ValueError('For builders who do not have attached devices, copying '
                       'from device to host is undefined and only allowed if '
                       'host_path and device_path are the same (%s vs %s).' % (
                       str(host_dir), str(device_dir)))  # pragma: no cover

  def copy_file_to_device(self, host_path, device_path):
    """Like shutil.copyfile, but for copying to a connected device."""
    # For "normal" builders who don't have an attached device, we expect
    # host_dir and device_dir to be the same.
    if str(host_path) != str(device_path):  # pragma: no cover
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
    self.m.file.makedirs(
        self.m.path.basename(path), path, infra_step=True)

  def install(self):
    """Run device-specific installation steps."""
    self.device_dirs = DeviceDirs(
        dm_dir=self.m.vars.dm_dir,
        perf_data_dir=self.m.vars.perf_data_dir,
        resource_dir=self.m.vars.resource_dir,
        images_dir=self.m.vars.images_dir,
        skp_dir=self.m.vars.local_skp_dir,
        svg_dir=self.m.vars.local_svg_dir,
        tmp_dir=self.m.vars.tmp_dir)

  def cleanup_steps(self):
    """Run any device-specific cleanup steps."""
    pass
