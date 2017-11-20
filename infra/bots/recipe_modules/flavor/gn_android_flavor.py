# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import recipe_api

import default_flavor
import re
import subprocess

ADB_BINARY = 'adb.1.0.35'

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

  def _run(self, title, *cmd, **kwargs):
    with self.m.context(cwd=self.m.vars.skia_dir):
      return self.m.run(self.m.step, title, cmd=list(cmd), **kwargs)

  def _py(self, title, script, infra_step=True):
    with self.m.context(cwd=self.m.vars.skia_dir):
      return self.m.run(self.m.python, title, script=script,
                        infra_step=infra_step)

  def _adb(self, title, *cmd, **kwargs):
    self._ever_ran_adb = True
    # The only non-infra adb steps (dm / nanobench) happen to not use _adb().
    if 'infra_step' not in kwargs:
      kwargs['infra_step'] = True

    attempts = 1
    flaky_devices = ['NexusPlayer', 'PixelC']
    if self.m.vars.builder_cfg.get('model') in flaky_devices:
      attempts = 3

    def wait_for_device(attempt):
      self.m.run(self.m.step,
                 'kill adb server after failure of \'%s\' (attempt %d)' % (
                     title, attempt),
                 cmd=[ADB_BINARY, 'kill-server'],
                 infra_step=True, timeout=30, abort_on_failure=False,
                 fail_build_on_failure=False)
      self.m.run(self.m.step,
                 'wait for device after failure of \'%s\' (attempt %d)' % (
                     title, attempt),
                 cmd=[ADB_BINARY, 'wait-for-device'], infra_step=True,
                 timeout=180, abort_on_failure=False,
                 fail_build_on_failure=False)

    with self.m.context(cwd=self.m.vars.skia_dir):
      return self.m.run.with_retry(self.m.step, title, attempts,
                                   cmd=[ADB_BINARY]+list(cmd),
                                   between_attempts_fn=wait_for_device,
                                   **kwargs)

  def _lock_cpu(self, target_percent):
    # adb root
    d = self._adb('root (to set cpu frequency)', 'root',
                  abort_on_failure=False, stdout=self.m.raw_io.output())
    if not d or d.retcode or 'production builds' in d.stdout:
      # ADB root failed
      return
    # get potential freqencies
    available_frequencies = self._adb(
        'fetch available frequencies', 'shell', 'cat '
        '/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies',
        stdout=self.m.raw_io.output()).stdout
    if available_frequencies and '/system/bin/sh' not in available_frequencies:
      available_frequencies = sorted(
          int(i) for i in available_frequencies.strip().split())
    else:
      # It's possibly an older kernel, like the Nexus 10. In that case,
      #query the min/max instead.
      scaling_min_freq = self._adb(
          'fetch min frequency', 'shell', 'cat '
          '/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq',
          stdout=self.m.raw_io.output())
      scaling_max_freq = self._adb(
          'fetch max frequency', 'shell', 'cat '
          '/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq',
          stdout=self.m.raw_io.output())
      if scaling_min_freq.stdout and scaling_max_freq.stdout:
        # The list of avaible frequencies is hidden, but if we select one that
        # doesn't exist, the device will pick the closest one that works.
        available_frequencies = [int(scaling_min_freq.stdout),
                                 int(scaling_max_freq.stdout)*target_percent,
                                 int(scaling_max_freq.stdout)]
      else:
        # Nothing we can do to set frequencies
        return


    # find the one closest to target percent
    maxfreq = available_frequencies[-1]
    target = int(round(maxfreq * target_percent))
    freq = maxfreq
    for f in reversed(available_frequencies[:-1]):
      if f <= target:
        freq = f
        break

    # set governer to userspace first, so frequency "takes"
    self._adb('Set cpu governer to userspace', 'shell', 'echo "userspace" > '
              '/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor')
    self._adb('Set frequency to %d' % freq, 'shell', 'echo %d > '
              '/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed' % freq)
    self._adb('Observed frequency after setting', 'shell', 'cat '
              '/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed')

  def compile(self, unused_target):
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

    # If an Android API level is specified, use that.
    m = re.search(r'API(\d+)', extra_config)
    if m and len(m.groups()) == 1:
      args['ndk_api'] = m.groups()[0]

    if extra_cflags:
      args['extra_cflags'] = repr(extra_cflags).replace("'", '"')

    gn_args = ' '.join('%s=%s' % (k,v) for (k,v) in sorted(args.iteritems()))

    gn    = 'gn.exe'    if 'Win' in os else 'gn'
    ninja = 'ninja.exe' if 'Win' in os else 'ninja'
    gn = self.m.vars.skia_dir.join('bin', gn)

    self._py('fetch-gn', self.m.vars.skia_dir.join('bin', 'fetch-gn'))
    self._run('gn gen', gn, 'gen', self.out_dir, '--args=' + gn_args)
    self._run('ninja', ninja, '-k', '0', '-C', self.out_dir)

  def install(self):
    self._adb('mkdir ' + self.device_dirs.resource_dir,
              'shell', 'mkdir', '-p', self.device_dirs.resource_dir)


  def cleanup_steps(self):
    if self._ever_ran_adb:
      self.m.run(self.m.python.inline, 'dump log', program="""
          import os
          import subprocess
          import sys
          out = sys.argv[1]
          log = subprocess.check_output(['%s', 'logcat', '-d'])
          for line in log.split('\\n'):
            tokens = line.split()
            if len(tokens) == 11 and tokens[-7] == 'F' and tokens[-3] == 'pc':
              addr, path = tokens[-2:]
              local = os.path.join(out, os.path.basename(path))
              if os.path.exists(local):
                sym = subprocess.check_output(['addr2line', '-Cfpe', local, addr])
                line = line.replace(addr, addr + ' ' + sym.strip())
            print line
          """ % ADB_BINARY,
          args=[self.m.vars.skia_out.join(self.m.vars.configuration)],
          infra_step=True,
          timeout=300,
          abort_on_failure=False)

    # Only shutdown the device and quarantine the bot if the first failed step
    # is an infra step. If, instead, we did this for any infra failures, we
    # would shutdown too much. For example, if a Nexus 10 died during dm
    # and the following pull step would also fail "device not found" - causing
    # us to run the shutdown command when the device was probably not in a
    # broken state; it was just rebooting.
    if (self.m.run.failed_steps and
        isinstance(self.m.run.failed_steps[0], recipe_api.InfraFailure)):
      self._adb('shut down device to quarantine bot', 'shell', 'reboot', '-p')

    if self._ever_ran_adb:
      self._adb('kill adb server', 'kill-server')

  def step(self, name, cmd, **kwargs):
    if (cmd[0] == 'nanobench'):
      self._lock_cpu(0.6)
    else:
      self._lock_cpu(1.0)
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
    subprocess.check_call(['%s', 'shell', 'sh', bin_dir + sh])
    try:
      sys.exit(int(subprocess.check_output(['%s', 'shell', 'cat',
                                            bin_dir + 'rc'])))
    except ValueError:
      print "Couldn't read the return code.  Probably killed for OOM."
      sys.exit(1)
    """ % (ADB_BINARY, ADB_BINARY), args=[self.m.vars.android_bin_dir, sh])

  def copy_file_to_device(self, host, device):
    self._adb('push %s %s' % (host, device), 'push', host, device)

  def copy_directory_contents_to_device(self, host, device):
    # Copy the tree, avoiding hidden directories and resolving symlinks.
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
        subprocess.check_call(['%s', 'push',
                               os.path.realpath(os.path.join(host, p, f)),
                               os.path.join(device, p, f)])
    """ % ADB_BINARY, args=[host, device], infra_step=True)

  def copy_directory_contents_to_host(self, device, host):
    self._adb('pull %s %s' % (device, host), 'pull', device, host)

  def read_file_on_device(self, path, **kwargs):
    rv = self._adb('read %s' % path,
                   'shell', 'cat', path, stdout=self.m.raw_io.output(),
                   **kwargs)
    return rv.stdout.rstrip() if rv and rv.stdout else None

  def remove_file_on_device(self, path):
    self._adb('rm %s' % path, 'shell', 'rm', '-f', path)

  def create_clean_device_dir(self, path):
    self._adb('rm %s' % path, 'shell', 'rm', '-rf', path)
    self._adb('mkdir %s' % path, 'shell', 'mkdir', '-p', path)
