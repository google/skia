#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import default_flavor
import os
import posixpath
import subprocess
import ssh_devices


"""Utils for running tests remotely over SSH."""


class SSHFlavorUtils(default_flavor.DefaultFlavorUtils):
  def __init__(self, *args, **kwargs):
    super(SSHFlavorUtils, self).__init__(*args, **kwargs)
    slave_info = ssh_devices.SLAVE_INFO.get(self._bot_info.slave_name,
                                            ssh_devices.SLAVE_INFO['default'])
    self._host = slave_info.ssh_host
    self._port = slave_info.ssh_port
    self._user = slave_info.ssh_user

  @property
  def host(self):
    return self._host

  @property
  def port(self):
    return self._port

  @property
  def user(self):
    return self._user

  def ssh(self, cmd, **kwargs):
    """Run the given SSH command."""
    ssh_cmd = ['ssh']
    if self.port:
      ssh_cmd.extend(['-p', self.port])
    dest = self.host
    if self.user:
      dest = self.user + '@' + dest
    ssh_cmd.append(dest)
    ssh_cmd.extend(cmd)
    return self._bot_info.run(ssh_cmd, **kwargs)

  def step(self, *args, **kwargs):
    """Run the given step over SSH."""
    self.ssh(*args, **kwargs)

  def device_path_join(self, *args):
    """Like os.path.join(), but for paths on a remote machine."""
    return posixpath.join(*args)

  def device_path_exists(self, path):  # pragma: no cover
    """Like os.path.exists(), but for paths on a remote device."""
    try:
      self.ssh(['test', '-e', path])
      return True
    except subprocess.CalledProcessError:
      return False

  def _remove_device_dir(self, path):
    """Remove the directory on the device."""
    self.ssh(['rm', '-rf', path])

  def _create_device_dir(self, path):
    """Create the directory on the device."""
    self.ssh(['mkdir', '-p', path])

  def create_clean_device_dir(self, path):
    """Like shutil.rmtree() + os.makedirs(), but on a remote device."""
    self._remove_device_dir(path)
    self._create_device_dir(path)

  def _make_scp_cmd(self, remote_path, recurse=True):
    """Prepare an SCP command.

    Returns a partial SCP command and an adjusted remote path.
    """
    cmd = ['scp']
    if recurse:
      cmd.append('-r')
    if self.port:
      cmd.extend(['-P', self.port])
    adj_remote_path = self.host + ':' + remote_path
    if self.user:
      adj_remote_path = self.user + '@' + adj_remote_path
    return cmd, adj_remote_path

  def copy_directory_contents_to_device(self, host_dir, device_dir):
    """Like shutil.copytree(), but for copying to a remote device."""
    _, remote_path = self._make_scp_cmd(device_dir)
    cmd = [os.path.join(self._bot_info.skia_dir, 'tools',
                        'scp_dir_contents.sh'),
           host_dir, remote_path]
    self._bot_info.run(cmd)

  def copy_directory_contents_to_host(self, device_dir, host_dir):
    """Like shutil.copytree(), but for copying from a remote device."""
    _, remote_path = self._make_scp_cmd(device_dir)
    cmd = [os.path.join(self._bot_info.skia_dir, 'tools',
                        'scp_dir_contents.sh'),
           remote_path, host_dir]
    self._bot_info.run(cmd)

  def copy_file_to_device(self, host_path, device_path):
    """Like shutil.copyfile, but for copying to a connected device."""
    cmd, remote_path = self._make_scp_cmd(device_path, recurse=False)
    cmd.extend([host_path, remote_path])
    self._bot_info.run(cmd)

  def read_file_on_device(self, path):
    return self.ssh(['cat', path]).rstrip()

  def remove_file_on_device(self, path):
    """Delete the given file."""
    return self.ssh(['rm', '-f', path])
