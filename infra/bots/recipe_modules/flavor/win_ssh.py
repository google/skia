# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api

import default
import json  # TODO(borenet): No! Remove this.
import ntpath
import subprocess  # TODO(borenet): No! Remove this.


"""Win SSH flavor, used for running code on Windows via an SSH connection.

Copied from chromebook.py and modified for Windows.
"""


class WinSSHFlavor(default.DefaultFlavor):

  def __init__(self, m):
    super(WinSSHFlavor, self).__init__(m)
    self._user_ip = ''

    self.remote_homedir = 'C:\\Users\\osman\\botdata\\'
    self.device_dirs = default.DeviceDirs(
      bin_dir       = self.remote_homedir + 'bin',
      dm_dir        = self.remote_homedir + 'dm_out',
      perf_data_dir = self.remote_homedir + 'perf',
      resource_dir  = self.remote_homedir + 'resources',
      images_dir    = self.remote_homedir + 'images',
      lotties_dir   = self.remote_homedir + 'lotties',
      skp_dir       = self.remote_homedir + 'skps',
      svg_dir       = self.remote_homedir + 'svgs',
      tmp_dir       = self.remote_homedir)
    self._empty_dir = self.device_dirs.tmp_dir + 'empty'


  @property
  def user_ip(self):
    if not self._user_ip:
      ssh_info = self.m.run(self.m.python.inline, 'read ssh_machine.json',
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

  def _cmd(self, title, cmd, infra_step=True, fail_errorlevel=1, **kwargs):
    return self.m.run(self.m.python, title,
                      script=self.module.resource('win_ssh_cmd.py'),
                      args=[self.user_ip, cmd, fail_errorlevel],
                      infra_step=infra_step, **kwargs)

  def _mkdir(self, path):
    self._cmd('mkdir %s' % path, 'if not exist "%s" md "%s"' % (path, path))

  def device_path_join(self, *args):
    return ntpath.join(*args)

  def install(self):
    self._mkdir(self.device_dirs.resource_dir)

    # Ensure that our empty dir is actually empty.
    self._cmd('rmdir %s' % self._empty_dir,
              'if exist "%s" rd "%s"' % (self._empty_dir, self._empty_dir))
    self._mkdir(self._empty_dir)

    self.create_clean_device_dir(self.device_dirs.bin_dir)

  def create_clean_device_dir(self, path):
    # Based on https://stackoverflow.com/a/98069 and
    # https://superuser.com/a/346112.
    self._cmd('clean %s' % path,
              'robocopy /mir "%s" "%s"' % (self._empty_dir, path),
              fail_errorlevel=8)

  def read_file_on_device(self, path, **kwargs):
    with self.m.step.nest('read %s' % path):
      with self.m.tempfile.temp_dir('read_file_on_device') as tmp:
        host_path = tmp.join('tmp')
        device_path = self._prefix_device_path(path)
        self._run('scp %s %s' % (device_path, host_path),
                  cmd=['scp', device_path, host_path], infra_step=True)
        return self.m.file.read_text('read %s' % host_path, host_path)

  def remove_file_on_device(self, path):
    self._cmd('rm %s' % path, 'if exist "%s" del "%s"' % (path, path))

  def _prefix_device_path(self, device_path):
    return '%s:%s' % (self.user_ip, device_path)

  def copy_file_to_device(self, host_path, device_path):
    device_path = self._prefix_device_path(device_path)
    self._run('scp %s %s' % (host_path, device_path),
              cmd=['scp', host_path, device_path], infra_step=True)

  def _copy_dir(self, src, dest):
    self._run('scp -r %s %s' % (src, dest),
              cmd=['scp', '-r', src, dest], infra_step=True)

  def copy_directory_contents_to_device(self, host_path, device_path):
    self._copy_dir(host_path, self._prefix_device_path(device_path))

  def copy_directory_contents_to_host(self, device_path, host_path):
    self._copy_dir(self._prefix_device_path(device_path), host_path)

  def step(self, name, cmd, infra_step=False, **kwargs):
    # Assume the binary always ends in ".exe".
    filename = cmd[0] + '.exe'
    host_path = self.host_dirs.bin_dir.join(filename)
    device_path = self.device_path_join(self.device_dirs.bin_dir, filename)

    # Copy the app to the device and run cmd.
    self.copy_file_to_device(host_path, device_path)
    cmd[0] = device_path
    self._cmd(name, subprocess.list2cmdline(map(str, cmd)),
              infra_step=infra_step, **kwargs)
