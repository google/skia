#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import default_flavor
import os
import subprocess


"""iOS flavor utils, used for building for and running tests on iOS."""


class iOSFlavorUtils(default_flavor.DefaultFlavorUtils):
  def __init__(self, bot_info):
    super(iOSFlavorUtils, self).__init__(bot_info)
    self.ios_bin = os.path.join(self._bot_info.skia_dir, 'platform_tools',
                                'ios', 'bin')

  def step(self, cmd, **kwargs):
    args = [os.path.join(self.ios_bin, 'ios_run_skia')]

    # Convert 'dm' and 'nanobench' from positional arguments
    # to flags, which is what iOSShell expects to select which
    # one is being run.
    cmd = ["--" + c if c in ['dm', 'nanobench'] else c
          for c in cmd]
    return self._bot_info.run(args + cmd, **kwargs)

  def compile(self, target):
    """Build the given target."""
    cmd = [os.path.join(self.ios_bin, 'ios_ninja')]
    self._bot_info.run(cmd)

  def device_path_join(self, *args):
    """Like os.path.join(), but for paths on a connected iOS device."""
    return '/'.join(args)

  def device_path_exists(self, path):
    """Like os.path.exists(), but for paths on a connected device."""
    return self._bot_info.run(
        [os.path.join(self.ios_bin, 'ios_path_exists'), path],
    ) # pragma: no cover

  def _remove_device_dir(self, path):
    """Remove the directory on the device."""
    return self._bot_info.run(
        [os.path.join(self.ios_bin, 'ios_rm'), path],
    )

  def _create_device_dir(self, path):
    """Create the directory on the device."""
    return self._bot_info.run(
        [os.path.join(self.ios_bin, 'ios_mkdir'), path],
    )

  def copy_directory_contents_to_device(self, host_dir, device_dir):
    """Like shutil.copytree(), but for copying to a connected device."""
    return self._bot_info.run([
        os.path.join(self.ios_bin, 'ios_push_if_needed'),
        host_dir, device_dir
    ])

  def copy_directory_contents_to_host(self, device_dir, host_dir):
    """Like shutil.copytree(), but for copying from a connected device."""
    return self._bot_info.run(
        [os.path.join(self.ios_bin, 'ios_pull_if_needed'),
             device_dir, host_dir],
    )

  def copy_file_to_device(self, host_path, device_path):
    """Like shutil.copyfile, but for copying to a connected device."""
    self._bot_info.run(
        [os.path.join(self.ios_bin, 'ios_push_file'), host_path, device_path],
    ) # pragma: no cover

  def create_clean_device_dir(self, path):
    """Like shutil.rmtree() + os.makedirs(), but on a connected device."""
    self._remove_device_dir(path)
    self._create_device_dir(path)

  def install(self):
    """Run device-specific installation steps."""
    self._bot_info.run([os.path.join(self.ios_bin, 'ios_install')])

  def cleanup_steps(self):
    """Run any device-specific cleanup steps."""
    self._bot_info.run([os.path.join(self.ios_bin, 'ios_restart')])
    self._bot_info.run(['sleep', '20'])

  def read_file_on_device(self, path):
    """Read the given file."""
    return subprocess.check_output(
        [os.path.join(self.ios_bin, 'ios_cat_file'), path]).rstrip()

  def remove_file_on_device(self, path):
    """Remove the file on the device."""
    return self._bot_info.run(
        [os.path.join(self.ios_bin, 'ios_rm'), path],
    )

  def get_device_dirs(self):
    """ Set the directories which will be used by the build steps."""
    prefix = self.device_path_join('skiabot', 'skia_')
    return default_flavor.DeviceDirs(
        dm_dir=prefix + 'dm',
        perf_data_dir=prefix + 'perf',
        resource_dir=prefix + 'resources',
        images_dir=prefix + 'images',
        skp_dir=prefix + 'skp/skps',
        tmp_dir=prefix + 'tmp_dir')
