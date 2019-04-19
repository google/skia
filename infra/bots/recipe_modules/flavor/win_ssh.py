# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api

import default
import ntpath
import ssh
import subprocess  # TODO(borenet): No! Remove this.


"""Win SSH flavor, used for running code on Windows via an SSH connection.

Copied from chromebook.py and modified for Windows.
"""


class WinSSHFlavor(ssh.SSHFlavor):

  def __init__(self, m):
    super(WinSSHFlavor, self).__init__(m)
    self.remote_homedir = 'C:\\Users\\chrome-bot\\botdata\\'
    self.device_dirs = default.DeviceDirs(
      bin_dir       = self.device_path_join(self.remote_homedir, 'bin'),
      dm_dir        = self.device_path_join(self.remote_homedir, 'dm_out'),
      perf_data_dir = self.device_path_join(self.remote_homedir, 'perf'),
      resource_dir  = self.device_path_join(self.remote_homedir, 'resources'),
      images_dir    = self.device_path_join(self.remote_homedir, 'images'),
      lotties_dir   = self.device_path_join(self.remote_homedir, 'lotties'),
      skp_dir       = self.device_path_join(self.remote_homedir, 'skps'),
      svg_dir       = self.device_path_join(self.remote_homedir, 'svgs'),
      tmp_dir       = self.remote_homedir)
    self._empty_dir = self.device_path_join(self.remote_homedir, 'empty')


  def _cmd(self, title, cmd, infra_step=True, fail_errorlevel=1, **kwargs):
    return self.m.run(self.m.python, title,
                      script=self.module.resource('win_ssh_cmd.py'),
                      args=[self.user_ip, cmd, fail_errorlevel],
                      infra_step=infra_step, **kwargs)

  def ensure_device_dir(self, path):
    self._cmd('mkdir %s' % path, 'if not exist "%s" md "%s"' % (path, path))

  def _rmdir(self, path):
    self._cmd('rmdir %s' % path, 'if exist "%s" rd "%s"' % (path, path))

  def device_path_join(self, *args):
    return ntpath.join(*args)

  def install(self):
    super(WinSSHFlavor, self).install()

    # Ensure that our empty dir is actually empty.
    self._rmdir(self._empty_dir)
    self.ensure_device_dir(self._empty_dir)

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
        host_path = tmp.join(ntpath.basename(path))
        device_path = self.scp_device_path(path)
        ok = self._run('scp %s %s' % (device_path, host_path),
                       cmd=['scp', device_path, host_path],
                       infra_step=True, **kwargs)
        # TODO(dogben): Should readfile respect fail_build_on_failure and
        # abort_on_failure?
        if ok:
          return self.m.run.readfile(host_path)

  def remove_file_on_device(self, path):
    self._cmd('rm %s' % path, 'if exist "%s" del "%s"' % (path, path))

  def _copy_dir(self, src, dest):
    self._run('scp -r %s %s' % (src, dest),
              cmd=['scp', '-r', src, dest], infra_step=True)

  def copy_directory_contents_to_device(self, host_path, device_path):
    # Callers expect that the destination directory is replaced, which is not
    # how scp works when the destination is a directory. Instead scp to tmp_dir
    # and then robocopy to the correct destination.
    # Other flavors use a glob and subprocess with shell=True to copy the
    # contents of host_path; however, there are a lot of ways POSIX shell
    # interpretation could mess up Windows path names.
    with self.m.step.nest('copy %s to device' % host_path):
      tmp_pardir = self.device_path_join(
          self.device_dirs.tmp_dir,
          'tmp_copy_directory_contents_to_device')
      self.create_clean_device_dir(tmp_pardir)
      tmpdir = self.device_path_join(tmp_pardir,
                                     self.m.path.basename(host_path))
      self._copy_dir(host_path, self.scp_device_path(tmpdir))
      self._cmd('copy %s to %s' % (tmpdir, device_path),
                'robocopy /mir "%s" "%s"' % (tmpdir, device_path),
                fail_errorlevel=8)

  def copy_directory_contents_to_host(self, device_path, host_path):
    # Note that the glob in src is interpreted by the remote shell.
    src = self.scp_device_path(self.device_path_join(device_path, '*'))
    self._copy_dir(src, host_path)

  def step(self, name, cmd, infra_step=False, **kwargs):
    # There may be DLLs in the same dir as the executable that must be loaded
    # (yes, Windows allows overriding system DLLs with files in the local
    # directory). For simplicity, just copy the entire dir to the device.
    self.copy_directory_contents_to_device(self.host_dirs.bin_dir,
                                           self.device_dirs.bin_dir)

    cmd[0] = self.device_path_join(self.device_dirs.bin_dir, cmd[0])
    self._cmd(name, subprocess.list2cmdline(map(str, cmd)),
              infra_step=infra_step, **kwargs)
