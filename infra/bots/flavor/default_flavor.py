#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Default flavor utils class, used for desktop bots."""


import os
import shutil
import sys


class DeviceDirs(object):
  def __init__(self,
               dm_dir,
               perf_data_dir,
               resource_dir,
               images_dir,
               skp_dir,
               tmp_dir):
    self._dm_dir = dm_dir
    self._perf_data_dir = perf_data_dir
    self._resource_dir = resource_dir
    self._images_dir = images_dir
    self._skp_dir = skp_dir
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
    return self._skp_dir

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
  def __init__(self, bot_info, *args, **kwargs):
    self._bot_info = bot_info
    self.chrome_path = os.path.join(os.path.expanduser('~'), 'src')

  def step(self, cmd, **kwargs):
    """Runs a step as appropriate for this flavor."""
    path_to_app = self._bot_info.out_dir.join(
        self._bot_info.configuration, cmd[0])
    if (sys.platform == 'linux' and
        'x86_64' in self._bot_info.bot_name and
        not 'TSAN' in self._bot_info.bot_name):
      new_cmd = ['catchsegv', path_to_app]
    else:
      new_cmd = [path_to_app]
    new_cmd.extend(cmd[1:])
    return self._bot_info.run(new_cmd, **kwargs)


  def compile(self, target):
    """Build the given target."""
    # The CHROME_PATH environment variable is needed for bots that use
    # toolchains downloaded by Chrome.
    env = {'CHROME_PATH': self.chrome_path}
    if sys.platform == 'win32':
      make_cmd = ['python', 'make.py']
    else:
      make_cmd = ['make']
    cmd = make_cmd + [target]
    self._bot_info.run(cmd, env=env)

  def device_path_join(self, *args):
    """Like os.path.join(), but for paths on a connected device."""
    return os.path.join(*args)

  def device_path_exists(self, path):
    """Like os.path.exists(), but for paths on a connected device."""
    return os.path.exists(path, infra_step=True)  # pragma: no cover

  def copy_directory_contents_to_device(self, host_dir, device_dir):
    """Like shutil.copytree(), but for copying to a connected device."""
    # For "normal" bots who don't have an attached device, we expect
    # host_dir and device_dir to be the same.
    if str(host_dir) != str(device_dir):
      raise ValueError('For bots who do not have attached devices, copying '
                       'from host to device is undefined and only allowed if '
                       'host_path and device_path are the same (%s vs %s).' % (
                       str(host_dir), str(device_dir)))  # pragma: no cover

  def copy_directory_contents_to_host(self, device_dir, host_dir):
    """Like shutil.copytree(), but for copying from a connected device."""
    # For "normal" bots who don't have an attached device, we expect
    # host_dir and device_dir to be the same.
    if str(host_dir) != str(device_dir):
      raise ValueError('For bots who do not have attached devices, copying '
                       'from device to host is undefined and only allowed if '
                       'host_path and device_path are the same (%s vs %s).' % (
                       str(host_dir), str(device_dir)))  # pragma: no cover

  def copy_file_to_device(self, host_path, device_path):
    """Like shutil.copyfile, but for copying to a connected device."""
    # For "normal" bots who don't have an attached device, we expect
    # host_dir and device_dir to be the same.
    if str(host_path) != str(device_path):  # pragma: no cover
      raise ValueError('For bots who do not have attached devices, copying '
                       'from host to device is undefined and only allowed if '
                       'host_path and device_path are the same (%s vs %s).' % (
                       str(host_path), str(device_path)))

  def create_clean_device_dir(self, path):
    """Like shutil.rmtree() + os.makedirs(), but on a connected device."""
    self.create_clean_host_dir(path)

  def create_clean_host_dir(self, path):
    """Convenience function for creating a clean directory."""
    shutil.rmtree(path)
    os.makedirs(path)

  def install(self):
    """Run device-specific installation steps."""
    pass

  def cleanup_steps(self):
    """Run any device-specific cleanup steps."""
    pass

  def get_device_dirs(self):
    """ Set the directories which will be used by the build steps.

    These refer to paths on the same device where the test executables will
    run, for example, for Android bots these are paths on the Android device
    itself. For desktop bots, these are just local paths.
    """
    join = lambda p: os.path.join(self._bot_info.build_dir, p)
    return DeviceDirs(
        dm_dir=join('dm'),
        perf_data_dir=self._bot_info.perf_data_dir,
        resource_dir=self._bot_info.resource_dir,
        images_dir=join('images'),
        skp_dir=self._bot_info.local_skp_dir,
        tmp_dir=join('tmp'))

  def __repr__(self):
    return '<%s object>' % self.__class__.__name__  # pragma: no cover
