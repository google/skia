# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api
from recipe_engine import recipe_test_api

from . import default
import subprocess  # TODO(borenet): No! Remove this.


"""Android flavor, used for running code on Android."""


class AndroidFlavor(default.DefaultFlavor):
  def __init__(self, m, app_name):
    super(AndroidFlavor, self).__init__(m, app_name)
    self._ever_ran_adb = False
    self.ADB_BINARY = '/usr/bin/adb.1.0.35'
    self.ADB_PUB_KEY = '/home/chrome-bot/.android/adbkey'
    if 'skia' not in self.m.vars.swarming_bot_id:
      self.ADB_BINARY = '/opt/infra-android/tools/adb'
      self.ADB_PUB_KEY = ('/home/chrome-bot/.android/'
                          'chrome_infrastructure_adbkey')

    # Data should go in android_data_dir, which may be preserved across runs.
    android_data_dir = '/sdcard/revenge_of_the_skiabot/'
    self.device_dirs = default.DeviceDirs(
        bin_dir        = '/data/local/tmp/',
        dm_dir         = android_data_dir + 'dm_out',
        perf_data_dir  = android_data_dir + 'perf',
        resource_dir   = android_data_dir + 'resources',
        fonts_dir      = 'NOT_SUPPORTED',
        images_dir     = android_data_dir + 'images',
        lotties_dir    = android_data_dir + 'lotties',
        skp_dir        = android_data_dir + 'skps',
        svg_dir        = android_data_dir + 'svgs',
        mskp_dir       = android_data_dir + 'mskp',
        tmp_dir        = android_data_dir,
        texttraces_dir = android_data_dir + 'text_blob_traces')

    # A list of devices we can't root.  If rooting fails and a device is not
    # on the list, we fail the task to avoid perf inconsistencies.
    self.cant_root = ['GalaxyS7_G930FD', 'GalaxyS9',
                      'GalaxyS20', 'MotoG4', 'NVIDIA_Shield',
                      'P30', 'Pixel4','Pixel4XL', 'Pixel5', 'TecnoSpark3Pro', 'JioNext']

    # Maps device type -> CPU ids that should be scaled for nanobench.
    # Many devices have two (or more) different CPUs (e.g. big.LITTLE
    # on Nexus5x). The CPUs listed are the biggest cpus on the device.
    # The CPUs are grouped together, so we only need to scale one of them
    # (the one listed) in order to scale them all.
    # E.g. Nexus5x has cpu0-3 as one chip and cpu4-5 as the other. Thus,
    # if one wants to run a single-threaded application (e.g. nanobench), one
    # can disable cpu0-3 and scale cpu 4 to have only cpu4 and 5 at the same
    # frequency.  See also disable_for_nanobench.
    self.cpus_to_scale = {
      'Nexus5x': [4],
      'Pixel': [2],
      'Pixel2XL': [4]
    }

    # Maps device type -> CPU ids that should be turned off when running
    # single-threaded applications like nanobench. The devices listed have
    # multiple, differnt CPUs. We notice a lot of noise that seems to be
    # caused by nanobench running on the slow CPU, then the big CPU. By
    # disabling this, we see less of that noise by forcing the same CPU
    # to be used for the performance testing every time.
    self.disable_for_nanobench = {
      'Nexus5x': range(0, 4),
      'Pixel': range(0, 2),
      'Pixel2XL': range(0, 4),
      'Pixel6': range(4,8), # Only use the 4 small cores.
      'Pixel7': range(4,8),
    }

    self.gpu_scaling = {
      "Nexus5":  450000000,
      "Nexus5x": 600000000,
    }

  def _wait_for_device(self, title, attempt):
    self.m.run(self.m.step,
                'adb kill-server after failure of \'%s\' (attempt %d)' % (
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
    self.m.run(self.m.step,
                'adb devices -l after failure of \'%s\' (attempt %d)' % (
                    title, attempt),
                cmd=[self.ADB_BINARY, 'devices', '-l'],
                infra_step=True, timeout=30, abort_on_failure=False,
                fail_build_on_failure=False)
    self.m.run(self.m.step,
                'adb reboot device after failure of \'%s\' (attempt %d)' % (
                    title, attempt),
                cmd=[self.ADB_BINARY, 'reboot'],
                infra_step=True, timeout=30, abort_on_failure=False,
                fail_build_on_failure=False)
    self.m.run(self.m.step,
                'wait for device after failure of \'%s\' (attempt %d)' % (
                    title, attempt),
                cmd=[
                    self.ADB_BINARY, 'wait-for-device', 'shell',
                    # Wait until the boot is actually complete.
                    # https://android.stackexchange.com/a/164050
                    'while [[ -z $(getprop sys.boot_completed) ]]; do sleep 1; done',
                ],
                timeout=180, abort_on_failure=False,
                fail_build_on_failure=False)
    device = self.m.vars.builder_cfg.get('model')
    if (device in self.cant_root): # pragma: nocover
      return
    self.m.run(self.m.step,
                'adb root',
                cmd=[
                  self.ADB_BINARY, 'root'
                ],
                timeout=180, abort_on_failure=False,
                fail_build_on_failure=False)

  def _adb(self, title, *cmd, **kwargs):
    # The only non-infra adb steps (dm / nanobench) happen to not use _adb().
    if 'infra_step' not in kwargs:
      kwargs['infra_step'] = True

    self._ever_ran_adb = True
    # ADB seems to be occasionally flaky on every device, so always retry.
    attempts = kwargs.pop('attempts', 3)

    def wait_for_device(attempt):
      return self._wait_for_device(title, attempt)

    with self.m.context(cwd=self.m.path.start_dir.join('skia')):
      with self.m.env({'ADB_VENDOR_KEYS': self.ADB_PUB_KEY}):
        return self.m.run.with_retry(self.m.step, title, attempts,
                                     cmd=[self.ADB_BINARY]+list(cmd),
                                     between_attempts_fn=wait_for_device,
                                     **kwargs)

  def _scale_for_dm(self):
    device = self.m.vars.builder_cfg.get('model')
    if (device in self.cant_root or
        self.m.vars.internal_hardware_label):
      return

    # This is paranoia... any CPUs we disabled while running nanobench
    # ought to be back online now that we've restarted the device.
    for i in self.disable_for_nanobench.get(device, []):
      self._set_cpu_online(i, 1) # enable

    scale_up = self.cpus_to_scale.get(device, [0])
    # For big.LITTLE devices, make sure we scale the LITTLE cores up;
    # there is a chance they are still in powersave mode from when
    # swarming slows things down for cooling down and charging.
    if 0 not in scale_up:
      scale_up.append(0)
    for i in scale_up:
      # AndroidOne doesn't support ondemand governor. hotplug is similar.
      if device == 'AndroidOne':
        self._set_governor(i, 'hotplug')
      elif device in ['Pixel3a', 'Pixel4', 'Pixel4a', 'Wembley', 'Pixel6', 'Pixel7']:
        # Pixel3a/4/4a have userspace powersave performance schedutil.
        # performance seems like a reasonable choice.
        self._set_governor(i, 'performance')
      else:
        self._set_governor(i, 'ondemand')

  def _scale_for_nanobench(self):
    device = self.m.vars.builder_cfg.get('model')
    if (device in self.cant_root or
      self.m.vars.internal_hardware_label):
      return

    # Set to 'powersave' for Pixel6 and Pixel7.
    for i in self.cpus_to_scale.get(device, [0]):
      if device in ['Pixel6', 'Pixel7']:
        self._set_governor(i, 'powersave')
      else:
        self._set_governor(i, 'userspace')
        self._scale_cpu(i, 0.6)

    for i in self.disable_for_nanobench.get(device, []):
      self._set_cpu_online(i, 0) # disable

    if device in self.gpu_scaling:
      #https://developer.qualcomm.com/qfile/28823/lm80-p0436-11_adb_commands.pdf
      # Section 3.2.1 Commands to put the GPU in performance mode
      # Nexus 5 is  320000000 by default
      # Nexus 5x is 180000000 by default
      gpu_freq = self.gpu_scaling[device]
      script = self.module.resource('set_gpu_scaling.py')
      self.m.run.with_retry(self.m.step,
        "Lock GPU to %d (and other perf tweaks)" % gpu_freq,
        3, # attempts
        cmd=['python3', script, self.ADB_BINARY, gpu_freq],
        infra_step=True,
        timeout=30)

  def _set_governor(self, cpu, gov):
    self._ever_ran_adb = True
    script = self.module.resource('set_cpu_scaling_governor.py')
    self.m.run.with_retry(self.m.step,
        "Set CPU %d's governor to %s" % (cpu, gov),
        3, # attempts
        cmd=['python3', script, self.ADB_BINARY, cpu, gov],
        infra_step=True,
        timeout=30)


  def _set_cpu_online(self, cpu, value):
    """Set /sys/devices/system/cpu/cpu{N}/online to value (0 or 1)."""
    self._ever_ran_adb = True
    msg = 'Disabling'
    if value:
      msg = 'Enabling'

    def wait_for_device(attempt):
      return self._wait_for_device("set cpu online", attempt) # pragma: nocover

    script = self.module.resource('set_cpu_online.py')
    self.m.run.with_retry(self.m.step,
        '%s CPU %d' % (msg, cpu),
        3, # attempts
        cmd=['python3', script, self.ADB_BINARY, cpu, value],
        infra_step=True,
        between_attempts_fn=wait_for_device,
        timeout=30)


  def _scale_cpu(self, cpu, target_percent):
    self._ever_ran_adb = True

    def wait_for_device(attempt):
      return self._wait_for_device("scale cpu", attempt)

    script = self.module.resource('scale_cpu.py')
    self.m.run.with_retry(self.m.step,
        'Scale CPU %d to %f' % (cpu, target_percent),
        3, # attempts
        cmd=['python3', script, self.ADB_BINARY, str(target_percent), cpu],
        infra_step=True,
        between_attempts_fn=wait_for_device,
        timeout=30)


  def _asan_setup_path(self):
    return self.m.vars.workdir.join(
        'android_ndk_linux', 'toolchains', 'llvm', 'prebuilt', 'linux-x86_64',
        'lib', 'clang', '17', 'bin', 'asan_device_setup')


  def install(self):
    self._adb('mkdir ' + self.device_dirs.resource_dir,
              'shell', 'mkdir', '-p', self.device_dirs.resource_dir)
    if self.m.vars.builder_cfg.get('model') in ['GalaxyS20', 'GalaxyS9']:
      # See skia:10184, should be moot once upgraded to Android 11?
      self._adb('cp libGLES_mali.so to ' + self.device_dirs.bin_dir,
                 'shell', 'cp',
                '/vendor/lib64/egl/libGLES_mali.so',
                self.device_dirs.bin_dir + 'libvulkan.so')
    if 'ASAN' in self.m.vars.extra_tokens:
      self._ever_ran_adb = True
      script = self.module.resource('setup_device_for_asan.py')
      self.m.run(
          self.m.step, 'Setting up device to run ASAN',
          cmd=['python3', script, self.ADB_BINARY, self._asan_setup_path()],
          infra_step=True,
          timeout=300,
          abort_on_failure=True)
    if self.app_name:
      if (self.app_name == 'nanobench'):
        self._scale_for_nanobench()
      else:
        self._scale_for_dm()
      app_path = self.host_dirs.bin_dir.join(self.app_name)
      self._adb('push %s' % self.app_name,
                'push', app_path, self.device_dirs.bin_dir)



  def cleanup_steps(self):
    self.m.run(self.m.step,
                'adb reboot device',
                cmd=[self.ADB_BINARY, 'reboot'],
                infra_step=True, timeout=30, abort_on_failure=False,
                fail_build_on_failure=False)
    self.m.run(self.m.step,
                'wait for device after rebooting',
                cmd=[
                    self.ADB_BINARY, 'wait-for-device', 'shell',
                    # Wait until the boot is actually complete.
                    # https://android.stackexchange.com/a/164050
                    'while [[ -z $(getprop sys.boot_completed) ]]; do sleep 1; done',
                ],
                timeout=180, abort_on_failure=False,
                fail_build_on_failure=False)

    if 'ASAN' in self.m.vars.extra_tokens:
      self._ever_ran_adb = True
      # Remove ASAN.
      self.m.run(self.m.step,
                 'wait for device before uninstalling ASAN',
                 cmd=[self.ADB_BINARY, 'wait-for-device', 'shell',
                 # Wait until the boot is actually complete.
                 # https://android.stackexchange.com/a/164050
                 'while [[ -z $(getprop sys.boot_completed) ]]; do sleep 1; done',
                 ], infra_step=True,
                 timeout=180, abort_on_failure=False,
                 fail_build_on_failure=False)
      self.m.run(self.m.step, 'uninstall ASAN',
                 cmd=[self._asan_setup_path(), '--revert'],
                 infra_step=True, timeout=300,
                 abort_on_failure=False, fail_build_on_failure=False)

    if self._ever_ran_adb:
      script = self.module.resource('dump_adb_log.py')
      self.m.run(self.m.step, 'dump log',
          cmd=['python3', script, self.host_dirs.bin_dir, self.ADB_BINARY],
          infra_step=True,
          timeout=300,
          abort_on_failure=False)

    # Only quarantine the bot if the first failed step
    # is an infra step. If, instead, we did this for any infra failures, we
    # would do this too much. For example, if a Nexus 10 died during dm
    # and the following pull step would also fail "device not found" - causing
    # us to run the shutdown command when the device was probably not in a
    # broken state; it was just rebooting.
    if (self.m.run.failed_steps and
        isinstance(self.m.run.failed_steps[0], recipe_api.InfraFailure)):
      bot_id = self.m.vars.swarming_bot_id
      self.m.file.write_text('Quarantining Bot',
                             '/home/chrome-bot/%s.force_quarantine' % bot_id,
                             ' ')

    # if self._ever_ran_adb:
    #   self._adb('kill adb server', 'kill-server')

  def step(self, name, cmd):
    sh = '%s.sh' % cmd[0]
    self.m.run.writefile(self.m.vars.tmp_dir.join(sh),
        'set -x; LD_LIBRARY_PATH=%s %s%s; echo $? >%src' % (
            self.device_dirs.bin_dir,
            self.device_dirs.bin_dir, subprocess.list2cmdline(map(str, cmd)),
            self.device_dirs.bin_dir))
    self._adb('push %s' % sh,
              'push', self.m.vars.tmp_dir.join(sh), self.device_dirs.bin_dir)

    self._adb('clear log', 'logcat', '-c')
    script = self.module.resource('run_sh.py')
    self.m.step('%s' % cmd[0],
        cmd=['python3', script, self.device_dirs.bin_dir, sh, self.ADB_BINARY])

  def copy_file_to_device(self, host, device):
    self._adb('push %s %s' % (host, device), 'push', host, device)

  def copy_directory_contents_to_device(self, host, device):
    contents = self.m.file.glob_paths('ls %s/*' % host,
                                      host, '*',
                                      test_data=['foo.png', 'bar.jpg'])
    args = contents + [device]
    self._adb('push %s/* %s' % (host, device), 'push', *args)

  def copy_directory_contents_to_host(self, device, host):
    # TODO(borenet): When all of our devices are on Android 6.0 and up, we can
    # switch to using tar to zip up the results before pulling.
    with self.m.step.nest('adb pull'):
      tmp = self.m.path.mkdtemp('adb_pull')
      self._adb('pull %s' % device, 'pull', device, tmp)
      paths = self.m.file.glob_paths(
          'list pulled files',
          tmp,
          self.m.path.basename(device) + self.m.path.sep + '*',
          test_data=['%d.png' % i for i in (1, 2)])
      for p in paths:
        self.m.file.copy('copy %s' % self.m.path.basename(p), p, host)

  def read_file_on_device(self, path, **kwargs):
    testKwargs = {
      'attempts': 1,
      'abort_on_failure': False,
      'fail_build_on_failure': False,
    }
    rv = self._adb('check if %s exists' % path,
                   'shell', 'test', '-f', path, **testKwargs)
    if not rv: # pragma: nocover
      return None

    rv = self._adb('read %s' % path,
                   'shell', 'cat', path, stdout=self.m.raw_io.output(),
                   **kwargs)
    return rv.stdout.decode('utf-8').rstrip() if rv and rv.stdout else None

  def remove_file_on_device(self, path):
    script = self.module.resource('remove_file_on_device.py')
    self.m.run.with_retry(self.m.step, 'rm %s' % path, 3,
        cmd=['python3', script, self.ADB_BINARY, path],
        infra_step=True)

  def create_clean_device_dir(self, path):
    self.remove_file_on_device(path)
    self._adb('mkdir %s' % path, 'shell', 'mkdir', '-p', path)
