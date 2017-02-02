# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


import copy
import default_flavor


"""iOS flavor utils, used for building for and running tests on iOS."""


class iOSFlavorUtils(default_flavor.DefaultFlavorUtils):
  def __init__(self, m):
    super(iOSFlavorUtils, self).__init__(m)
    self.default_env = {}
    self.default_env['XCODEBUILD'] = (
        self.m.vars.slave_dir.join('xcodebuild'))
    self.ios_bin = self.m.vars.skia_dir.join(
        'platform_tools', 'ios', 'bin')

  def step(self, name, cmd, **kwargs):
    args = [self.ios_bin.join('ios_run_skia')]
    env = {}
    env.update(kwargs.pop('env', {}))
    env.update(self.default_env)
    # Convert 'dm' and 'nanobench' from positional arguments
    # to flags, which is what iOSShell expects to select which
    # one is being run.
    cmd = ["--" + c if c in ['dm', 'nanobench'] else c
          for c in cmd]
    return self.m.run(self.m.step, name=name, cmd=args + cmd,
                            env=env, **kwargs)

  def compile(self, target, **kwargs):
    """Build the given target."""
    cmd = [self.ios_bin.join('ios_ninja')]
    self.m.run(self.m.step, 'build iOSShell', cmd=cmd,
               cwd=self.m.path['checkout'], **kwargs)

  def device_path_join(self, *args):
    """Like os.path.join(), but for paths on a connected iOS device."""
    return '/'.join(args)

  def _remove_device_dir(self, path):
    """Remove the directory on the device."""
    return self.m.run(
        self.m.step,
        'rmdir %s' % path,
        cmd=[self.ios_bin.join('ios_rm'), path],
        env=self.default_env,
        infra_step=True,
    )

  def _create_device_dir(self, path):
    """Create the directory on the device."""
    return self.m.run(
        self.m.step,
        'mkdir %s' % path,
        cmd=[self.ios_bin.join('ios_mkdir'), path],
        env=self.default_env,
        infra_step=True,
    )

  def copy_directory_contents_to_device(self, host_dir, device_dir):
    """Like shutil.copytree(), but for copying to a connected device."""
    return self.m.run(
        self.m.step,
        name='push %s to %s' % (self.m.path.basename(host_dir),
                                self.m.path.basename(device_dir)),
        cmd=[self.ios_bin.join('ios_push_if_needed'),
             host_dir, device_dir],
        env=self.default_env,
        infra_step=True,
    )

  def copy_directory_contents_to_host(self, device_dir, host_dir):
    """Like shutil.copytree(), but for copying from a connected device."""
    self.m.run(
        self.m.step,
        name='pull %s' % self.m.path.basename(device_dir),
        cmd=[self.ios_bin.join('ios_pull_if_needed'),
             device_dir, host_dir],
        env=self.default_env,
        infra_step=True,
    )

  def copy_file_to_device(self, host_path, device_path):
    """Like shutil.copyfile, but for copying to a connected device."""
    self.m.run(
        self.m.step,
        name='push %s' % host_path,
        cmd=[self.ios_bin.join('ios_push_file'), host_path, device_path],
        env=self.default_env,
        infra_step=True,
    )

  def copy_extra_build_products(self, swarming_out_dir):
    xcode_dir = self.m.path.join(
        'xcodebuild', '%s-iphoneos' % self.m.vars.configuration)
    self.m.run.copy_build_products(
        self.m.vars.skia_dir.join(xcode_dir),
        swarming_out_dir.join(xcode_dir))

  def create_clean_device_dir(self, path):
    """Like shutil.rmtree() + os.makedirs(), but on a connected device."""
    self._remove_device_dir(path)
    self._create_device_dir(path)

  def install(self):
    """Run device-specific installation steps."""
    prefix = self.device_path_join('skiabot', 'skia_')
    self.device_dirs = default_flavor.DeviceDirs(
        dm_dir=prefix + 'dm',
        perf_data_dir=prefix + 'perf',
        resource_dir=prefix + 'resources',
        images_dir=prefix + 'images',
        skp_dir=prefix + 'skp/skps',
        svg_dir=prefix + 'svg/svgs',
        tmp_dir=prefix + 'tmp_dir')

    self.m.run(
        self.m.step,
        name='install iOSShell',
        cmd=[self.ios_bin.join('ios_install')],
        env=self.default_env,
        infra_step=True)

  def cleanup_steps(self):
    """Run any device-specific cleanup steps."""
    if self.m.vars.role in (self.m.builder_name_schema.BUILDER_ROLE_TEST,
                            self.m.builder_name_schema.BUILDER_ROLE_PERF):
      self.m.run(
          self.m.step,
          name='reboot',
          cmd=[self.ios_bin.join('ios_restart')],
          env=self.default_env,
          infra_step=True)
      self.m.run(
          self.m.step,
          name='wait for reboot',
          cmd=['sleep', '20'],
          env=self.default_env,
          infra_step=True)

  def read_file_on_device(self, path):
    """Read the given file."""
    ret = self.m.run(
        self.m.step,
        name='read %s' % self.m.path.basename(path),
        cmd=[self.ios_bin.join('ios_cat_file'), path],
        env=self.default_env,
        stdout=self.m.raw_io.output(),
        infra_step=True)
    return ret.stdout.rstrip() if ret.stdout else ret.stdout

  def remove_file_on_device(self, path):
    """Remove the file on the device."""
    return self.m.run(
        self.m.step,
        'rm %s' % path,
        cmd=[self.ios_bin.join('ios_rm'), path],
        env=self.default_env,
        infra_step=True,
    )
