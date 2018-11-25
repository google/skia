# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import recipe_api

import default_flavor
import gn_android_flavor
import subprocess


"""GN Chromecast flavor utils, used for building Skia for Chromecast with GN"""
class GNChromecastFlavorUtils(gn_android_flavor.GNAndroidFlavorUtils):
  def __init__(self, m):
    super(GNChromecastFlavorUtils, self).__init__(m)
    self._ever_ran_adb = False
    self._user_ip = ''
    self.m.vars.android_bin_dir = self.m.path.join(self.m.vars.android_bin_dir,
                                                   'bin')

    # Disk space is extremely tight on the Chromecasts (~100M) There is not
    # enough space on the android_data_dir (/cache/skia) to fit the images,
    # resources, executable and output the dm images.  So, we have dm_out be
    # on the tempfs (i.e. RAM) /dev/shm. (which is about 140M)

    self.device_dirs = default_flavor.DeviceDirs(
        dm_dir        = '/dev/shm/skia/dm_out',
        perf_data_dir = self.m.vars.android_data_dir + 'perf',
        resource_dir  = self.m.vars.android_data_dir + 'resources',
        images_dir    = self.m.vars.android_data_dir + 'images',
        skp_dir       = self.m.vars.android_data_dir + 'skps',
        svg_dir       = self.m.vars.android_data_dir + 'svgs',
        tmp_dir       = self.m.vars.android_data_dir)

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

  def compile(self, unused_target):
    configuration = self.m.vars.builder_cfg.get('configuration')
    os            = self.m.vars.builder_cfg.get('os')
    target_arch   = self.m.vars.builder_cfg.get('target_arch')

    # TODO(kjlubick): can this toolchain be replaced/shared with chromebook?
    toolchain_dir = self.m.vars.slave_dir.join('cast_toolchain', 'armv7a')
    gles_dir = self.m.vars.slave_dir.join('chromebook_arm_gles')

    extra_cflags = [
      '-I%s' % gles_dir.join('include'),
      '-DMESA_EGL_NO_X11_HEADERS',
      "-DSK_NO_COMMAND_BUFFER",
      # Avoid unused warning with yyunput
      '-Wno-error=unused-function',
      # Makes the binary small enough to fit on the small disk.
      '-g0',
    ]

    extra_ldflags = [
      # Chromecast does not package libstdc++
      '-static-libstdc++', '-static-libgcc',
      '-L%s' % toolchain_dir.join('lib'),
    ]

    quote = lambda x: '"%s"' % x
    args = {
      'cc': quote(toolchain_dir.join('bin','armv7a-cros-linux-gnueabi-gcc')),
      'cxx': quote(toolchain_dir.join('bin','armv7a-cros-linux-gnueabi-g++')),
      'ar': quote(toolchain_dir.join('bin','armv7a-cros-linux-gnueabi-ar')),
      'target_cpu': quote(target_arch),
      'skia_use_fontconfig': 'false',
      'skia_enable_gpu': 'true',
      # The toolchain won't allow system libraries to be used
      # when cross-compiling
      'skia_use_system_freetype2': 'false',
      # Makes the binary smaller
      'skia_use_icu': 'false',
      'skia_use_egl': 'true',
    }

    if configuration != 'Debug':
      args['is_debug'] = 'false'
    args['extra_cflags'] = repr(extra_cflags).replace("'", '"')
    args['extra_ldflags'] = repr(extra_ldflags).replace("'", '"')

    gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.iteritems()))

    gn    = 'gn.exe'    if 'Win' in os else 'gn'
    ninja = 'ninja.exe' if 'Win' in os else 'ninja'
    gn = self.m.vars.skia_dir.join('bin', gn)

    self._py('fetch-gn', self.m.vars.skia_dir.join('bin', 'fetch-gn'))
    self._run('gn gen', gn, 'gen', self.out_dir, '--args=' + gn_args)
    self._run('ninja', ninja, '-k', '0', '-C', self.out_dir, 'nanobench', 'dm')

  def install(self):
    super(GNChromecastFlavorUtils, self).install()
    self._adb('mkdir ' + self.m.vars.android_bin_dir,
              'shell', 'mkdir', '-p', self.m.vars.android_bin_dir)

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
      self._ssh('Delete executables', 'rm', '-r', self.m.vars.android_bin_dir,
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
          args=[self.m.vars.skia_out.join(self.m.vars.configuration)],
          infra_step=True,
          abort_on_failure=False)

      self._adb('disconnect', 'disconnect')
      self._adb('kill adb server', 'kill-server')

  def _ssh(self, title, *cmd, **kwargs):
    ssh_cmd = ['ssh', '-oConnectTimeout=15', '-oBatchMode=yes',
               '-t', '-t', 'root@%s' % self.user_ip] + list(cmd)

    return self.m.run(self.m.step, title, cmd=ssh_cmd, **kwargs)

  def step(self, name, cmd, **kwargs):
    app = self.m.vars.skia_out.join(self.m.vars.configuration, cmd[0])

    self._adb('push %s' % cmd[0],
              'push', app, self.m.vars.android_bin_dir)

    cmd[0] = '%s/%s' % (self.m.vars.android_bin_dir, cmd[0])
    self._ssh(str(name), *cmd, infra_step=False)

