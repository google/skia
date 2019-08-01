# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Disable warning about setting self.device_dirs in install(); we need to.
# pylint: disable=W0201


from . import default


"""iOS flavor, used for running code on iOS."""


class iOSFlavor(default.DefaultFlavor):
  def __init__(self, m):
    super(iOSFlavor, self).__init__(m)
    self.device_dirs = default.DeviceDirs(
        bin_dir='[unused]',
        dm_dir='dm',
        perf_data_dir='perf',
        resource_dir='resources',
        images_dir='images',
        lotties_dir='lotties',
        skp_dir='skps',
        svg_dir='svgs',
        mskp_dir='mskp',
        tmp_dir='tmp')

  def install(self):
    # Set up the device
    self.m.run(self.m.step, 'setup_device', cmd=['ios.py'], infra_step=True)

    # Install the app.
    for app_name in ['dm', 'nanobench']:
      app_package = self.host_dirs.bin_dir.join('%s.app' % app_name)

      def uninstall_app(attempt):
        # If app ID changes, upgrade will fail, so try uninstalling.
        self.m.run(self.m.step,
                   'uninstall_' + app_name,
                   cmd=['ideviceinstaller', '-U', 'com.google.%s' % app_name],
                   infra_step=True,
                   # App may not be installed.
                   abort_on_failure=False, fail_build_on_failure=False)

      num_attempts = 2
      self.m.run.with_retry(self.m.step, 'install_' + app_name, num_attempts,
                            cmd=['ideviceinstaller', '-i', app_package],
                            between_attempts_fn=uninstall_app,
                            infra_step=True)

  def step(self, name, cmd, env=None, **kwargs):
    bundle_id = 'com.google.%s' % cmd[0]
    self.m.run(self.m.step, name,
               cmd=['idevice-app-runner', '-s', bundle_id, '--args'] +
                    map(str, cmd[1:]))

  def _run_ios_script(self, script, first, *rest):
    full = self.m.path['start_dir'].join(
        'skia', 'platform_tools', 'ios', 'bin', 'ios_' + script)
    self.m.run(self.m.step,
               name = '%s %s' % (script, first),
               cmd = [full, first] + list(rest),
               infra_step=True)

  def copy_file_to_device(self, host, device):
    self._run_ios_script('push_file', host, device)

  def copy_directory_contents_to_device(self, host, device):
    self._run_ios_script('push_if_needed', host, device)

  def copy_directory_contents_to_host(self, device, host):
    self._run_ios_script('pull_if_needed', device, host)

  def remove_file_on_device(self, path):
    self._run_ios_script('rm', path)

  def create_clean_device_dir(self, path):
    self._run_ios_script('rm',    path)
    self._run_ios_script('mkdir', path)

  def read_file_on_device(self, path, **kwargs):
    full = self.m.path['start_dir'].join(
        'skia', 'platform_tools', 'ios', 'bin', 'ios_cat_file')
    rv = self.m.run(self.m.step,
                    name = 'cat_file %s' % path,
                    cmd = [full, path],
                    stdout=self.m.raw_io.output(),
                    infra_step=True,
                    **kwargs)
    return rv.stdout.rstrip() if rv and rv.stdout else None
