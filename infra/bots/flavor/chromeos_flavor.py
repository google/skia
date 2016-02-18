#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import default_flavor
import os
import ssh_flavor


"""Utils for building for and running tests on ChromeOS."""


class ChromeOSFlavorUtils(ssh_flavor.SSHFlavorUtils):
  def __init__(self, bot_info):
    super(ChromeOSFlavorUtils, self).__init__(bot_info)
    self.board = self._bot_info.spec['device_cfg']
    self.device_root_dir = '/usr/local/skiabot'
    self.device_bin_dir = self.device_path_join(self.device_root_dir, 'bin')

  def step(self, name, cmd, **kwargs):
    """Wrapper for the Step API; runs a step as appropriate for this flavor."""
    local_path = self._bot_info.out_dir.join(
      'config', 'chromeos-%s' % self.board,
      self._bot_info.configuration, cmd[0])
    remote_path = self.device_path_join(self.device_bin_dir, cmd[0])
    self.copy_file_to_device(local_path, remote_path)
    super(ChromeOSFlavorUtils, self).step(name=name,
                                          cmd=[remote_path]+cmd[1:],
                                          **kwargs)

  def compile(self, target):
    """Build the given target."""
    cmd = [os.path.join(self._bot_info.skia_dir, 'platform_tools', 'chromeos',
                        'bin', 'chromeos_make'),
           '-d', self.board,
           target]
    self._bot_info.run(cmd)

  def install(self):
    """Run any device-specific installation steps."""
    self.create_clean_device_dir(self.device_bin_dir)

  def get_device_dirs(self):
    """ Set the directories which will be used by the build steps."""
    prefix = self.device_path_join(self.device_root_dir, 'skia_')
    def join(suffix):
      return ''.join((prefix, suffix))
    return default_flavor.DeviceDirs(
        dm_dir=join('dm_out'),  # 'dm' conflicts with the binary
        perf_data_dir=join('perf'),
        resource_dir=join('resources'),
        images_dir=join('images'),
        skp_dir=self.device_path_join(join('skp'), 'skps'),
        tmp_dir=join('tmp_dir'))

