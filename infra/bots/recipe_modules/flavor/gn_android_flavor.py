# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from recipe_engine import recipe_api

import default_flavor
import re
import subprocess


"""GN Android flavor utils, used for building Skia for Android with GN."""
class GNAndroidFlavorUtils(default_flavor.DefaultFlavorUtils):
  def __init__(self, m):
    super(GNAndroidFlavorUtils, self).__init__(m)
    self._ever_ran_adb = False
    self.ADB_BINARY = '/usr/bin/adb.1.0.35'
    golo_devices = ['Nexus5x']
    if self.m.vars.builder_cfg.get('model') in golo_devices:
      self.ADB_BINARY = '/opt/infra-android/tools/adb'

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
    # The only non-infra adb steps (dm / nanobench) happen to not use _adb().
    if 'infra_step' not in kwargs:
      kwargs['infra_step'] = True

    self._ever_ran_adb = True
    attempts = 1
    flaky_devices = ['NexusPlayer', 'PixelC']
    if self.m.vars.builder_cfg.get('model') in flaky_devices:
      attempts = 3

    def wait_for_device(attempt):
      self.m.run(self.m.step,
                 'kill adb server after failure of \'%s\' (attempt %d)' % (
                     title, attempt),
                 cmd=[self.ADB_BINARY, 'kill-server'],
                 infra_step=True, timeout=30, abort_on_failure=False,
                 fail_build_on_failure=False)
      self.m.run(self.m.step,
                 'wait for device after failure of \'%s\' (attempt %d)' % (
                     title, attempt),
                 cmd=[self.ADB_BINARY, 'wait-for-device'], infra_step=True,
                 timeout=180, abort_on_failure=False,
                 fail_build_on_failure=False)

    with self.m.context(cwd=self.m.vars.skia_dir):
      return self.m.run.with_retry(self.m.step, title, attempts,
                                   cmd=[self.ADB_BINARY]+list(cmd),
                                   between_attempts_fn=wait_for_device,
                                   **kwargs)

  # A list of devices we can't root.  If rooting fails and a device is not
  # on the list, we fail the task to avoid perf inconsistencies.
  rootable_blacklist = ['GalaxyS6', 'GalaxyS7_G930A', 'GalaxyS7_G930FD',
                        'MotoG4', 'NVIDIA_Shield']

  # Maps device type -> cpu ids that need to be set when scaling, with the
  # primary CPU being listed first. CPUs are configured together,
  # for example, the Nexus 5x has cpu0-3 and cpu4-5 linked together, each
  # group using the same configuration - anything that happens to cpu0,
  # happens to cpu1-3.
  # cpus at index 1+ will be clocked down to powersave during nanobench.
  # TODO(kjlubick): determine this dynamically - should be able to look
  # at which cores have the highest frequency and affected_cpus.
  cpus_to_scale = {
    'Nexus5x': [4, 0],
    'NexusPlayer': [2, 0],
    'Pixel': [2, 0],
    'Pixel2XL': [4, 0]
  }

  # Maps device -> number of cores. TODO(kjlubick) if we want to do this
  # long term, compute this dynamically.
  total_cpus = {
    'AndroidOne': 4,
    'Nexus5': 4,
    'Nexus7': 4,
    'Nexus5x': 6,
    'NexusPlayer': 4,
    'Pixel': 4,
    'Pixel2XL': 8,
    'PixelC': 4,
  }

  def _scale_for_dm(self):
    device = self.m.vars.builder_cfg.get('model')
    if (device in self.rootable_blacklist or
        self.m.vars.internal_hardware_label):
      return

    for i in self.cpus_to_scale.get(device, [0]):
      # AndroidOne doesn't support ondemand governor. hotplug is similar.
      if device == 'AndroidOne':
        self._set_governor(i, 'hotplug')
      else:
        self._set_governor(i, 'ondemand')

  def _scale_for_nanobench(self):
    device = self.m.vars.builder_cfg.get('model')
    if (device in self.rootable_blacklist or
      self.m.vars.internal_hardware_label):
      return

    # Scale just the first two cpus.
    self._set_governor(0, 'userspace')
    self._scale_cpu(0, 0.6)

    for i in range(2, self.total_cpus[device]):
      # NexusPlayer only has "ondemand userspace interactive performance"
      self._disable_cpu(i)


  def _set_governor(self, cpu, gov):
    self._ever_ran_adb = True
    self.m.run.with_retry(self.m.python.inline,
        "Set CPU %d's governor to %s" % (cpu, gov),
        3, # attempts
        program="""
import os
import subprocess
import sys
import time
ADB = sys.argv[1]
cpu = int(sys.argv[2])
gov = sys.argv[3]

log = subprocess.check_output([ADB, 'root'])
# check for message like 'adbd cannot run as root in production builds'
print log
if 'cannot' in log:
  raise Exception('adb root failed')

subprocess.check_output([ADB, 'shell', 'echo "%s" > '
    '/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor' % (gov, cpu)])
actual_gov = subprocess.check_output([ADB, 'shell', 'cat '
    '/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor' % cpu]).strip()
if actual_gov != gov:
  raise Exception('(actual, expected) (%s, %s)'
                  % (actual_gov, gov))
""",
        args = [self.ADB_BINARY, cpu, gov],
        infra_step=True,
        timeout=30)


  def _disable_cpu(self, cpu):
    self._ever_ran_adb = True
    self.m.run.with_retry(self.m.python.inline,
        'Disabling CPU %d' % cpu,
        3, # attempts
        program="""
import os
import subprocess
import sys
import time
ADB = sys.argv[1]
cpu = int(sys.argv[2])

log = subprocess.check_output([ADB, 'root'])
# check for message like 'adbd cannot run as root in production builds'
print log
if 'cannot' in log:
  raise Exception('adb root failed')

subprocess.check_output([ADB, 'shell', 'echo 0 > '
    '/sys/devices/system/cpu/cpu%d/online' % cpu])
actual_status = subprocess.check_output([ADB, 'shell', 'cat '
    '/sys/devices/system/cpu/cpu%d/online' % cpu]).strip()
if actual_status != str(0):
  raise Exception('(actual, expected) (%s, %d)'
                  % (actual_gov, gov))
""",
        args = [self.ADB_BINARY, cpu],
        infra_step=True,
        timeout=30)


  def _scale_cpu(self, cpu, target_percent):
    self._ever_ran_adb = True
    self.m.run.with_retry(self.m.python.inline,
        'Scale CPU %d to %f' % (cpu, target_percent),
        3, # attempts
        program="""
import os
import subprocess
import sys
import time
ADB = sys.argv[1]
target_percent = float(sys.argv[2])
cpu = int(sys.argv[3])
log = subprocess.check_output([ADB, 'root'])
# check for message like 'adbd cannot run as root in production builds'
print log
if 'cannot' in log:
  raise Exception('adb root failed')

root = '/sys/devices/system/cpu/cpu%d/cpufreq' %cpu

# All devices we test on give a list of their available frequencies.
available_freqs = subprocess.check_output([ADB, 'shell',
    'cat %s/scaling_available_frequencies' % root])

# Check for message like '/system/bin/sh: file not found'
if available_freqs and '/system/bin/sh' not in available_freqs:
  available_freqs = sorted(
      int(i) for i in available_freqs.strip().split())
else:
  raise Exception('Could not get list of available frequencies: %s' %
                  available_freqs)

maxfreq = available_freqs[-1]
target = int(round(maxfreq * target_percent))
freq = maxfreq
for f in reversed(available_freqs):
  if f <= target:
    freq = f
    break

print 'Setting frequency to %d' % freq

# If scaling_max_freq is lower than our attempted setting, it won't take.
# We must set min first, because if we try to set max to be less than min
# (which sometimes happens after certain devices reboot) it returns a
# perplexing permissions error.
subprocess.check_output([ADB, 'shell', 'echo 0 > '
    '%s/scaling_min_freq' % root])
subprocess.check_output([ADB, 'shell', 'echo %d > '
    '%s/scaling_max_freq' % (freq, root)])
subprocess.check_output([ADB, 'shell', 'echo %d > '
    '%s/scaling_setspeed' % (freq, root)])
time.sleep(5)
actual_freq = subprocess.check_output([ADB, 'shell', 'cat '
    '%s/scaling_cur_freq' % root]).strip()
if actual_freq != str(freq):
  raise Exception('(actual, expected) (%s, %d)'
                  % (actual_freq, freq))
""",
        args = [self.ADB_BINARY, str(target_percent), cpu],
        infra_step=True,
        timeout=30)

  def compile(self, unused_target):
    compiler      = self.m.vars.builder_cfg.get('compiler')
    configuration = self.m.vars.builder_cfg.get('configuration')
    extra_tokens  = self.m.vars.extra_tokens
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
    if 'Vulkan' in extra_tokens:
      args['ndk_api'] = 24
      args['skia_enable_vulkan_debug_layers'] = 'false'

    # If an Android API level is specified, use that.
    for t in extra_tokens:
      m = re.search(r'API(\d+)', t)
      if m and len(m.groups()) == 1:
        args['ndk_api'] = m.groups()[0]
        break

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
          """ % self.ADB_BINARY,
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
      self._scale_for_nanobench()
    else:
      self._scale_for_dm()
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
    """ % (self.ADB_BINARY, self.ADB_BINARY),
      args=[self.m.vars.android_bin_dir, sh])

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
    """ % self.ADB_BINARY, args=[host, device], infra_step=True)

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
