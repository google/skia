# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api

import default_flavor
import gn_flavor
import json
import subprocess


"""
  GN Chromebook flavor utils, used for building and testing Skia for ARM
  Chromebooks with GN
"""
class GNChromebookFlavorUtils(gn_flavor.GNFlavorUtils):

  def __init__(self, m):
    super(GNChromebookFlavorUtils, self).__init__(m)
    self._user_ip = ''

    self.device_dirs = default_flavor.DeviceDirs(
      dm_dir        = self.m.vars.chromeos_homedir + 'dm_out',
      perf_data_dir = self.m.vars.chromeos_homedir + 'perf',
      resource_dir  = self.m.vars.chromeos_homedir + 'resources',
      images_dir    = self.m.vars.chromeos_homedir + 'images',
      skp_dir       = self.m.vars.chromeos_homedir + 'skps',
      svg_dir       = self.m.vars.chromeos_homedir + 'svgs',
      tmp_dir       = self.m.vars.chromeos_homedir)

    self._bin_dir = self.m.vars.chromeos_homedir + 'bin'

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
    self._ssh('remount %s as exec' % self.m.vars.chromeos_homedir,
              'sudo', 'mount', '-i', '-o', 'remount,exec', '/home/chronos')

    self.create_clean_device_dir(self._bin_dir)

  def compile(self, unused_target):
    configuration = self.m.vars.builder_cfg.get('configuration')
    os            = self.m.vars.builder_cfg.get('os')
    target_arch   = self.m.vars.builder_cfg.get('target_arch')

    clang_linux = self.m.vars.slave_dir.join('clang_linux')
    # This is a pretty typical arm-linux-gnueabihf sysroot
    sysroot_dir = self.m.vars.slave_dir.join('armhf_sysroot')
    # This is the extra things needed to link against for the chromebook.
    #  For example, the Mali GL drivers.
    gl_dir   = self.m.vars.slave_dir.join('chromebook_arm_gles')

    extra_asmflags = [
      '--target=armv7a-linux-gnueabihf',
      '--sysroot=%s' % sysroot_dir,
      '-march=armv7-a',
      '-mfpu=neon',
      '-mthumb',
    ]

    extra_cflags = [
      '--target=armv7a-linux-gnueabihf',
      '--sysroot=%s' % sysroot_dir,
      '-I%s' % gl_dir.join('include'),
      '-I%s' % sysroot_dir.join('include'),
      '-I%s' % sysroot_dir.join('include', 'c++', '4.8.4'),
      '-I%s' % sysroot_dir.join('include', 'c++', '4.8.4',
                                'arm-linux-gnueabihf'),
      '-DMESA_EGL_NO_X11_HEADERS',
    ]

    extra_ldflags = [
      '--target=armv7a-linux-gnueabihf',
      '--sysroot=%s' % sysroot_dir,
      # use sysroot's ld which can properly link things.
      '-B%s' % sysroot_dir.join('bin'),
      # helps locate crt*.o
      '-B%s' % sysroot_dir.join('gcc-cross'),
      # helps locate libgcc*.so
      '-L%s' % sysroot_dir.join('gcc-cross'),
      '-L%s' % sysroot_dir.join('lib'),
      '-L%s' % gl_dir.join('lib'),
      # Explicitly do not use lld for cross compiling like this - I observed
      # failures like "Unrecognized reloc 41" and couldn't find out why.
    ]

    quote = lambda x: '"%s"' % x
    args = {
      'cc': quote(clang_linux.join('bin','clang')),
      'cxx': quote(clang_linux.join('bin','clang++')),
      'target_cpu': quote(target_arch),
      'skia_use_fontconfig': 'false',
      'skia_use_system_freetype2': 'false',
      'skia_use_egl': 'true',
    }

    if configuration != 'Debug':
      args['is_debug'] = 'false'
    args['extra_asmflags'] = repr(extra_asmflags).replace("'", '"')
    args['extra_cflags'] = repr(extra_cflags).replace("'", '"')
    args['extra_ldflags'] = repr(extra_ldflags).replace("'", '"')

    gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.iteritems()))

    gn    = 'gn.exe'    if 'Win' in os else 'gn'
    ninja = 'ninja.exe' if 'Win' in os else 'ninja'
    gn = self.m.vars.skia_dir.join('bin', gn)

    with self.m.context(cwd=self.m.vars.skia_dir,
                        env={'LD_LIBRARY_PATH': sysroot_dir.join('lib')}):
      self._py('fetch-gn', self.m.vars.skia_dir.join('bin', 'fetch-gn'))
      self._run('gn gen', [gn, 'gen', self.out_dir, '--args=' + gn_args])
      self._run('ninja', [ninja, '-C', self.out_dir, 'nanobench', 'dm'])

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
    app = self.m.vars.skia_out.join(self.m.vars.configuration, cmd[0])

    cmd[0] = '%s/%s' % (self._bin_dir, cmd[0])
    self.copy_file_to_device(app, cmd[0])

    self._ssh('chmod %s' % name, 'chmod', '+x', cmd[0])
    self._ssh(str(name), *cmd)
