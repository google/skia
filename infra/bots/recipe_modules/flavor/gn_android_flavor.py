# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import default_flavor
import subprocess

"""GN Android flavor utils, used for building Skia for Android with GN."""
class GNAndroidFlavorUtils(default_flavor.DefaultFlavorUtils):
  def __init__(self, m):
    super(GNAndroidFlavorUtils, self).__init__(m)
    self._ever_ran_adb = False

    prefix = '/data/local/tmp/'
    self.device_dirs = default_flavor.DeviceDirs(
        dm_dir        = prefix + 'dm_out',
        perf_data_dir = prefix + 'perf',
        resource_dir  = prefix + 'resources',
        images_dir    = prefix + 'images',
        skp_dir       = prefix + 'skps',
        svg_dir       = prefix + 'svgs',
        tmp_dir       = prefix + 'tmp')

  def supported(self):
    return 'GN_Android' == self.m.vars.builder_cfg.get('extra_config', '')

  def _run(self, title, *cmd, **kwargs):
    self.m.vars.default_env = {k: v for (k,v)
                               in self.m.vars.default_env.iteritems()
                               if k in ['PATH']}
    return self.m.run(self.m.step, title, cmd=list(cmd),
                      cwd=self.m.vars.skia_dir, env={}, **kwargs)

  def _adb(self, title, *cmd, **kwargs):
    self._ever_ran_adb = True
    return self._run(title, 'adb', *cmd, **kwargs)

  def compile(self, unused_target, **kwargs):
    compiler      = self.m.vars.builder_cfg.get('compiler')
    configuration = self.m.vars.builder_cfg.get('configuration')
    os            = self.m.vars.builder_cfg.get('os')
    target_arch   = self.m.vars.builder_cfg.get('target_arch')

    assert compiler == 'Clang'  # At this rate we might not ever support GCC.

    ndk_asset = 'android_ndk_linux' if os == 'Ubuntu' else 'android_ndk_darwin'

    quote = lambda x: '"%s"' % x
    gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted({
        'is_debug': 'true' if configuration == 'Debug' else 'false',
        'ndk': quote(self.m.vars.slave_dir.join(ndk_asset)),
        'target_cpu': quote(target_arch),
    }.iteritems()))

    self._run('fetch-gn', self.m.vars.skia_dir.join('bin', 'fetch-gn'))
    self._run('gn gen', 'gn', 'gen', self.out_dir, '--args=' + gn_args)
    self._run('ninja', 'ninja', '-C', self.out_dir)

  def install(self):
    self._adb('reboot', 'reboot')
    self._adb('wait for device', 'wait-for-usb-device')
    self._adb('TEMPORARY clear /data/local/tmp',
              'shell', 'rm', '-rf', '/data/local/tmp/*')
    self._adb('mkdir /data/local/tmp/resources',
              'shell', 'mkdir', '-p', '/data/local/tmp/resources')

  def cleanup_steps(self):
    if self._ever_ran_adb:
      self._adb('TEMPORARY clear /data/local/tmp',
                'shell', 'rm', '-rf', '/data/local/tmp/*')
      self._adb('TEMPORARY reboot', 'reboot')
      self._adb('kill adb server', 'kill-server')

  def step(self, name, cmd, env=None, **kwargs):
    app = self.m.vars.skia_out.join(self.m.vars.configuration, cmd[0])
    self._adb('push %s' % cmd[0],
              'push', app, '/data/local/tmp')

    sh = '%s.sh' % cmd[0]
    self.m.run.writefile(self.m.vars.tmp_dir.join(sh),
        'set -x; /data/local/tmp/%s; echo $? >/data/local/tmp/rc' %
        subprocess.list2cmdline(map(str, cmd)))
    self._adb('push %s' % sh,
              'push', self.m.vars.tmp_dir.join(sh), '/data/local/tmp')

    self._adb('clear log', 'logcat', '-c')
    self._adb(cmd[0], 'shell', 'sh', '/data/local/tmp/' + sh)
    self._adb('dump log ', 'logcat', '-d')

    self.m.python.inline('check %s rc' % cmd[0], """
    import subprocess
    import sys
    sys.exit(int(subprocess.check_output(['adb', 'shell', 'cat',
                                          '/data/local/tmp/rc'])))
    """)

  def copy_file_to_device(self, host, device):
    self._adb('push %s %s' % (host, device), 'push', host, device)

  def copy_directory_contents_to_device(self, host, device):
    # Copy the tree, avoiding hidden directories and resolving symlinks.
    self.m.python.inline('push %s/* %s' % (host, device), """
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
        subprocess.check_call(['adb', 'push',
                               os.path.realpath(os.path.join(host, p, f)),
                               os.path.join(device, p, f)])
    """, args=[host, device], cwd=self.m.vars.skia_dir)

  def copy_directory_contents_to_host(self, device, host):
    self._adb('pull %s %s' % (device, host), 'pull', device, host)

  def read_file_on_device(self, path):
    return self._adb('read %s' % path,
                     'shell', 'cat', path, stdout=self.m.raw_io.output()).stdout

  def remove_file_on_device(self, path):
    self._adb('rm %s' % path, 'shell', 'rm', '-f', path)

  def create_clean_device_dir(self, path):
    self._adb('rm %s' % path, 'shell', 'rm', '-rf', path)
    self._adb('mkdir %s' % path, 'shell', 'mkdir', '-p', path)
