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

    self.device_dirs = default_flavor.DeviceDirs(
        dm_dir        = self.m.vars.android_data_dir + 'dm_out',
        perf_data_dir = self.m.vars.android_data_dir + 'perf',
        resource_dir  = self.m.vars.android_data_dir + 'resources',
        images_dir    = self.m.vars.android_data_dir + 'images',
        skp_dir       = self.m.vars.android_data_dir + 'skps',
        svg_dir       = self.m.vars.android_data_dir + 'svgs',
        tmp_dir       = self.m.vars.android_data_dir)

  def _strip_environment(self):
    self.m.vars.default_env = {k: v for (k,v)
                               in self.m.vars.default_env.iteritems()
                               if k in ['PATH']}

  def _run(self, title, *cmd, **kwargs):
    self._strip_environment()
    return self.m.run(self.m.step, title, cmd=list(cmd),
                      cwd=self.m.vars.skia_dir, **kwargs)

  def _py(self, title, script, infra_step=True):
    self._strip_environment()
    return self.m.run(self.m.python, title, script=script,
                      cwd=self.m.vars.skia_dir, env=None, infra_step=infra_step)

  def _adb(self, title, *cmd, **kwargs):
    self._ever_ran_adb = True
    # The only non-infra adb steps (dm / nanobench) happen to not use _adb().
    if 'infra_step' not in kwargs:
      kwargs['infra_step'] = True
    return self._run(title, 'adb', *cmd, **kwargs)

  def compile(self, unused_target, **kwargs):
    compiler      = self.m.vars.builder_cfg.get('compiler')
    configuration = self.m.vars.builder_cfg.get('configuration')
    extra_config  = self.m.vars.builder_cfg.get('extra_config', '')
    os            = self.m.vars.builder_cfg.get('os')
    target_arch   = self.m.vars.builder_cfg.get('target_arch')

    assert compiler == 'Clang'  # At this rate we might not ever support GCC.

    extra_cflags = []
    if configuration == 'Debug':
      extra_cflags.append('-O1')

    ndk_asset = 'android_ndk_linux'
    if 'Mac' in os:
      ndk_asset = 'android_ndk_darwin'
    elif 'Win' in os:
      ndk_asset = 'n'

    quote = lambda x: '"%s"' % x
    args = {
        'ndk': quote(self.m.vars.slave_dir.join(ndk_asset)),
        'target_cpu': quote(target_arch),
    }

    if configuration != 'Debug':
      args['is_debug'] = 'false'
    if 'Vulkan' in extra_config:
      args['ndk_api'] = 24
      args['skia_enable_vulkan_debug_layers'] = 'false'
    if 'FrameworkDefs' in extra_config:
      args['skia_enable_android_framework_defines'] = 'true'
    if extra_cflags:
      args['extra_cflags'] = repr(extra_cflags).replace("'", '"')

    gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.iteritems()))

    gn    = 'gn.bat'    if 'Win' in os else 'gn'
    ninja = 'ninja.exe' if 'Win' in os else 'ninja'

    self._py('fetch-gn', self.m.vars.skia_dir.join('bin', 'fetch-gn'))
    self._run('gn gen', gn, 'gen', self.out_dir, '--args=' + gn_args)
    self._run('ninja', ninja, '-C', self.out_dir)

  def install(self):
    self._adb('mkdir ' + self.device_dirs.resource_dir,
              'shell', 'mkdir', '-p', self.device_dirs.resource_dir)

  def cleanup_steps(self):
    if self._ever_ran_adb:
      self.m.python.inline('dump log', """
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
      infra_step=True)
      self._adb('kill adb server', 'kill-server')

  def step(self, name, cmd, env=None, **kwargs):
    app = self.m.vars.skia_out.join(self.m.vars.configuration, cmd[0])
    self._adb('push %s' % cmd[0],
              'push', app, self.m.vars.android_bin_dir)

    sh = '%s.sh' % cmd[0]
    self.m.run.writefile(self.m.vars.tmp_dir.join(sh),
        'set -x; %s%s; echo $? >%src' %
        (self.m.vars.android_bin_dir, subprocess.list2cmdline(map(str, cmd)),
            self.m.vars.android_bin_dir))
    self._adb('push %s' % sh,
              'push', self.m.vars.tmp_dir.join(sh), self.m.vars.android_bin_dir)

    self._adb('clear log', 'logcat', '-c')
    self.m.python.inline('%s' % cmd[0], """
    import subprocess
    import sys
    bin_dir = sys.argv[1]
    sh      = sys.argv[2]
    subprocess.check_call(['adb', 'shell', 'sh', bin_dir + sh])
    try:
      sys.exit(int(subprocess.check_output(['adb', 'shell', 'cat',
                                            bin_dir + 'rc'])))
    except ValueError:
      print "Couldn't read the return code.  Probably killed for OOM."
      sys.exit(1)
    """, args=[self.m.vars.android_bin_dir, sh])

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
    """, args=[host, device], cwd=self.m.vars.skia_dir, infra_step=True)

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
