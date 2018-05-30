# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api

import default
import json  # TODO(borenet): No! Remove this.


"""Chromebook flavor, used for running code on Chromebooks."""


class ChromebookFlavor(default.DefaultFlavor):

  def __init__(self, m):
    super(ChromebookFlavor, self).__init__(m)
    self._user_ip = ''

    self.chromeos_homedir = '/home/chronos/user/'
    self.device_dirs = default.DeviceDirs(
      bin_dir       = self.chromeos_homedir + 'bin',
      dm_dir        = self.chromeos_homedir + 'dm_out',
      perf_data_dir = self.chromeos_homedir + 'perf',
      resource_dir  = self.chromeos_homedir + 'resources',
      images_dir    = self.chromeos_homedir + 'images',
      skp_dir       = self.chromeos_homedir + 'skps',
      svg_dir       = self.chromeos_homedir + 'svgs',
      tmp_dir       = self.chromeos_homedir)

  @property
  def user_ip(self):
    if not self._user_ip:
      ssh_info = self.m.run(self.m.python.inline, 'read chromeos ip',
                            program="""
      import os
      SSH_MACHINE_FILE = os.path.expanduser('~/ssh_machine.json')
      with open(SSH_MACHINE_FILE, 'r') as f:
        print f.read()
      """,
      stdout=self.m.raw_io.output(),
      infra_step=True).stdout

      self._user_ip = json.loads(ssh_info).get(u'user_ip', 'ERROR')
    return self._user_ip

  def _ssh(self, title, *cmd, **kwargs):
    if 'infra_step' not in kwargs:
      kwargs['infra_step'] = True

    ssh_cmd = ['ssh', '-oConnectTimeout=15', '-oBatchMode=yes',
               '-t', '-t', self.user_ip] + list(cmd)

    return self._run(title, ssh_cmd, **kwargs)

  def install(self):
    self._ssh('mkdir %s' % self.device_dirs.resource_dir, 'mkdir', '-p',
              self.device_dirs.resource_dir)

    # Ensure the home dir is marked executable
    self._ssh('remount %s as exec' % self.chromeos_homedir,
              'sudo', 'mount', '-i', '-o', 'remount,exec', '/home/chronos')

    self.create_clean_device_dir(self.device_dirs.bin_dir)

  def create_clean_device_dir(self, path):
    # use -f to silently return if path doesn't exist
    self._ssh('rm %s' % path, 'rm', '-rf', path)
    self._ssh('mkdir %s' % path, 'mkdir', '-p', path)

  def read_file_on_device(self, path, **kwargs):
    rv = self._ssh('read %s' % path,
                   'cat', path, stdout=self.m.raw_io.output(),
                   **kwargs)
    return rv.stdout.rstrip() if rv and rv.stdout else None

  def remove_file_on_device(self, path):
    # use -f to silently return if path doesn't exist
    self._ssh('rm %s' % path, 'rm', '-f', path)

  def _prefix_device_path(self, device_path):
    return '%s:%s' % (self.user_ip, device_path)

  def copy_file_to_device(self, host_path, device_path):
    device_path = self._prefix_device_path(device_path)
    # Recipe
    self.m.python.inline(str('scp %s %s' % (host_path, device_path)),
    """
    import subprocess
    import sys
    host = sys.argv[1]
    device   = sys.argv[2]
    print subprocess.check_output(['scp', host, device])
    """, args=[host_path, device_path], infra_step=True)

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
    self._copy_dir(host_path, self._prefix_device_path(device_path))

  def copy_directory_contents_to_host(self, device_path, host_path):
    self._copy_dir(self._prefix_device_path(device_path), host_path)

  def step(self, name, cmd, **kwargs):
    # Push and run either dm or nanobench

    name = cmd[0]

    if name == 'dm':
      self.create_clean_host_dir(self.host_dirs.dm_dir)
    if name == 'nanobench':
      self.create_clean_host_dir(self.host_dirs.perf_data_dir)

    app = self.host_dirs.bin_dir.join(cmd[0])

    cmd[0] = '%s/%s' % (self.device_dirs.bin_dir, cmd[0])
    self.copy_file_to_device(app, cmd[0])

    self._ssh('chmod %s' % name, 'chmod', '+x', cmd[0])
    self._ssh(str(name), *cmd)
