# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api

import default


"""SSH flavor, used for running code on a remote device via SSH.

Must be subclassed to set self.device_dirs. The default implementation assumes
a Linux-based device.
"""


class SSHFlavor(default.DefaultFlavor):

  def __init__(self, m, app_name):
    super(SSHFlavor, self).__init__(m, app_name)
    self._user_ip = ''

  @property
  def user_ip(self):
    if not self._user_ip:
      path = '/tmp/ssh_machine.json'
      ssh_info = self.m.file.read_json('read ssh_machine.json', path,
                                       test_data={'user_ip':'foo@127.0.0.1'})
      self._user_ip = ssh_info.get(u'user_ip')
    return self._user_ip

  def ssh(self, title, *cmd, **kwargs):
    if 'infra_step' not in kwargs:
      kwargs['infra_step'] = True

    ssh_cmd = ['ssh', '-oConnectTimeout=15', '-oBatchMode=yes',
               '-t', '-t', self.user_ip] + list(cmd)

    return self._run(title, ssh_cmd, **kwargs)

  def ensure_device_dir(self, path):
    self.ssh('mkdir %s' % path, 'mkdir', '-p', path)

  def install(self):
    self.ensure_device_dir(self.device_dirs.resource_dir)
    if self.app_name:
      self.create_clean_device_dir(self.device_dirs.bin_dir)
      host_path = self.host_dirs.bin_dir.join(self.app_name)
      device_path = self.device_path_join(self.device_dirs.bin_dir, self.app_name)
      self.copy_file_to_device(host_path, device_path)
      self.ssh('make %s executable' % self.app_name, 'chmod', '+x', device_path)

  def create_clean_device_dir(self, path):
    # use -f to silently return if path doesn't exist
    self.ssh('rm %s' % path, 'rm', '-rf', path)
    self.ensure_device_dir(path)

  def read_file_on_device(self, path, **kwargs):
    rv = self.ssh('read %s' % path,
                   'cat', path, stdout=self.m.raw_io.output(),
                   **kwargs)
    return rv.stdout.rstrip() if rv and rv.stdout else None

  def remove_file_on_device(self, path):
    # use -f to silently return if path doesn't exist
    self.ssh('rm %s' % path, 'rm', '-f', path)

  def scp_device_path(self, device_path):
    return '%s:%s' % (self.user_ip, device_path)

  def copy_file_to_device(self, host_path, device_path):
    device_path = self.scp_device_path(device_path)
    self._run('scp %s %s' % (host_path, device_path),
              ['scp', host_path, device_path], infra_step=True)

  # TODO(benjaminwagner): implement with rsync
  #def copy_directory_contents_to_device(self, host_path, device_path):

  # TODO(benjaminwagner): implement with rsync
  #def copy_directory_contents_to_host(self, device_path, host_path):

  def step(self, name, cmd, **kwargs):
    # Run cmd (installed above)
    cmd[0] = self.device_path_join(self.device_dirs.bin_dir, cmd[0])
    self.ssh(str(name), *cmd)
