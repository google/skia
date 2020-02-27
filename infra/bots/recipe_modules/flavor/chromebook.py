# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api

import default
import ssh


"""Chromebook flavor, used for running code on Chromebooks."""


class ChromebookFlavor(ssh.SSHFlavor):

  def __init__(self, m, app_name):
    super(ChromebookFlavor, self).__init__(m, app_name)
    self.chromeos_homedir = '/home/chronos/user/'
    self.device_dirs = default.DeviceDirs(
      bin_dir        = self.chromeos_homedir + 'bin',
      dm_dir         = self.chromeos_homedir + 'dm_out',
      perf_data_dir  = self.chromeos_homedir + 'perf',
      resource_dir   = self.chromeos_homedir + 'resources',
      images_dir     = self.chromeos_homedir + 'images',
      lotties_dir    = self.chromeos_homedir + 'lotties',
      skp_dir        = self.chromeos_homedir + 'skps',
      svg_dir        = self.chromeos_homedir + 'svgs',
      mskp_dir       = self.chromeos_homedir + 'mskp',
      tmp_dir        = self.chromeos_homedir,
      texttraces_dir = '')

  def install(self):
    super(ChromebookFlavor, self).install()

    # Ensure the home dir is marked executable
    self.ssh('remount %s as exec' % self.chromeos_homedir,
             'sudo', 'mount', '-i', '-o', 'remount,exec', '/home/chronos')

  def _copy_dir(self, src, dest):
    # We can't use rsync to communicate with the chromebooks because the
    # chromebooks don't have rsync installed on them.
    self.m.python.inline(str('scp -r %s %s' % (src, dest)),
    """
    import subprocess
    import sys
    src = sys.argv[1] + '/*'
    dest   = sys.argv[2]
    print subprocess.check_output('scp -r %s %s' % (src, dest), shell=True)
    """, args=[src, dest], infra_step=True)

  def copy_directory_contents_to_device(self, host_path, device_path):
    self._copy_dir(host_path, self.scp_device_path(device_path))

  def copy_directory_contents_to_host(self, device_path, host_path):
    self._copy_dir(self.scp_device_path(device_path), host_path)
