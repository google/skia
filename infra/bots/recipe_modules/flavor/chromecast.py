# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import recipe_api

from . import android
from . import default


"""Chromecast flavor, used for running code on Chromecast"""


class ChromecastFlavor(android.AndroidFlavor):
  def __init__(self, m):
    super(ChromecastFlavor, self).__init__(m)
    self._ever_ran_adb = False
    self._user_ip = ''

    # Disk space is extremely tight on the Chromecasts (~100M) There is not
    # enough space on the android_data_dir (/cache/skia) to fit the images,
    # resources, executable and output the dm images.  So, we have dm_out be
    # on the tempfs (i.e. RAM) /dev/shm. (which is about 140M)
    data_dir = '/cache/skia/'
    self.device_dirs = default.DeviceDirs(
        bin_dir       = '/cache/skia/bin',
        dm_dir        = '/dev/shm/skia/dm_out',
        perf_data_dir = data_dir + 'perf',
        resource_dir  = data_dir + 'resources',
        images_dir    = data_dir + 'images',
        lotties_dir   = data_dir + 'lotties',
        skp_dir       = data_dir + 'skps',
        svg_dir       = data_dir + 'svgs',
        mskp_dir      = data_dir + 'mskp',
        tmp_dir       = data_dir)

  @property
  def user_ip_host(self):
    if not self._user_ip:
      self._user_ip = self.m.run(self.m.python.inline, 'read chromecast ip',
                                 program="""
      import os
      CHROMECAST_IP_FILE = os.path.expanduser('~/chromecast.txt')
      with open(CHROMECAST_IP_FILE, 'r') as f:
        print f.read()
      """,
      stdout=self.m.raw_io.output(),
      infra_step=True).stdout

    return self._user_ip

  @property
  def user_ip(self):
    return self.user_ip_host.split(':')[0]

  def install(self):
    super(ChromecastFlavor, self).install()
    self._adb('mkdir ' + self.device_dirs.bin_dir,
              'shell', 'mkdir', '-p', self.device_dirs.bin_dir)

  def _adb(self, title, *cmd, **kwargs):
    if not self._ever_ran_adb:
      self._connect_to_remote()

    self._ever_ran_adb = True
    # The only non-infra adb steps (dm / nanobench) happen to not use _adb().
    if 'infra_step' not in kwargs:
      kwargs['infra_step'] = True
    return self._run(title, 'adb', *cmd, **kwargs)

  def _connect_to_remote(self):
    self.m.run(self.m.step, 'adb connect %s' % self.user_ip_host, cmd=['adb',
      'connect', self.user_ip_host], infra_step=True)

  def create_clean_device_dir(self, path):
    # Note: Chromecast does not support -rf
    self._adb('rm %s' % path, 'shell', 'rm', '-r', path)
    self._adb('mkdir %s' % path, 'shell', 'mkdir', '-p', path)

  def copy_directory_contents_to_device(self, host, device):
    # Copy the tree, avoiding hidden directories and resolving symlinks.
    # Additionally, due to space restraints, we don't push files > 3 MB
    # which cuts down the size of the SKP asset to be around 50 MB as of
    # version 41.
    self.m.run(self.m.python.inline, 'push %s/* %s' % (host, device),
               program="""
    import os
    import subprocess
    import sys
    host   = sys.argv[1]
    device = sys.argv[2]
    for d, _, fs in os.walk(host):
      p = os.path.relpath(d, host)
      if p != '.' and p.startswith('.'):
        continue
      for f in fs:
        print os.path.join(p,f)
        hp = os.path.realpath(os.path.join(host, p, f))
        if os.stat(hp).st_size > (1.5 * 1024 * 1024):
          print "Skipping because it is too big"
        else:
          subprocess.check_call(['adb', 'push',
                                hp, os.path.join(device, p, f)])
    """, args=[host, device], infra_step=True)

  def cleanup_steps(self):
    if self._ever_ran_adb:
      # To clean up disk space for next time
      self._ssh('Delete executables', 'rm', '-r', self.device_dirs.bin_dir,
                abort_on_failure=False, infra_step=True)
      # Reconnect if was disconnected
      self._adb('disconnect', 'disconnect')
      self._connect_to_remote()
      self.m.run(self.m.python.inline, 'dump log', program="""
          import os
          import subprocess
          import sys
          out = sys.argv[1]
          log = subprocess.check_output(['adb', 'logcat', '-d'])
          for line in log.split('\\n'):
            tokens = line.split()
            if len(tokens) == 11 and tokens[-7] == 'F' and tokens[-3] == 'pc':
              addr, path = tokens[-2:]
              local = os.path.join(out, os.path.basename(path))
              if os.path.exists(local):
                sym = subprocess.check_output(['addr2line', '-Cfpe', local, addr])
                line = line.replace(addr, addr + ' ' + sym.strip())
            print line
          """,
          args=[self.host_dirs.bin_dir],
          infra_step=True,
          abort_on_failure=False)

      self._adb('disconnect', 'disconnect')
      self._adb('kill adb server', 'kill-server')

  def _ssh(self, title, *cmd, **kwargs):
    # Don't use -t -t (Force psuedo-tty allocation) like in the ChromeOS
    # version because the pseudo-tty allocation seems to fail
    # instantly when talking to a Chromecast.
    # This was excacerbated when we migrated to kitchen and was marked by
    # the symptoms of all the ssh commands instantly failing (even after
    # connecting and authenticating) with exit code -1 (255)
    ssh_cmd = ['ssh', '-oConnectTimeout=15', '-oBatchMode=yes',
               '-T', 'root@%s' % self.user_ip] + list(cmd)

    return self.m.run(self.m.step, title, cmd=ssh_cmd, **kwargs)

  def step(self, name, cmd, **kwargs):
    app = self.host_dirs.bin_dir.join(cmd[0])

    self._adb('push %s' % cmd[0],
              'push', app, self.device_dirs.bin_dir)

    cmd[0] = '%s/%s' % (self.device_dirs.bin_dir, cmd[0])
    self._ssh(str(name), *cmd, infra_step=False)
