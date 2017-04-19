# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Disable warning about setting self.device_dirs in install(); we need to.
# pylint: disable=W0201

import default_flavor
import gn_flavor
import os

# Infra step failures interact really annoyingly with swarming retries.
kInfraStep = False

class iOSFlavorUtils(gn_flavor.GNFlavorUtils):

  def install(self):
    self.device_dirs = default_flavor.DeviceDirs(
        dm_dir='dm',
        perf_data_dir='perf',
        resource_dir='resources',
        images_dir='images',
        skp_dir='skps',
        svg_dir='svgs',
        tmp_dir='tmp')

  def compile(self, unused_target, **kwargs):
    """ Build Skia with GN and sign the iOS apps"""
    # Use the generic compile sets.
    super(iOSFlavorUtils, self).compile(unused_target, **kwargs)

    # Sign the apps.
    for app in ['dm', 'nanobench']:
      self._py('package ' + app,
              self.m.vars.skia_dir.join('gn', 'package_ios.py'),
              args=[self.out_dir.join(app)])

  def step(self, name, cmd, env=None, **kwargs):
    app = self.m.vars.skia_out.join(self.m.vars.configuration, cmd[0])

    self._run(name,
              ['ios-deploy', '-b', '%s.app' % app,
               '-I', '--args', ' '.join(map(str, cmd[1:]))])

  def _run_ios_script(self, script, first, *rest):
    full = self.m.vars.skia_dir.join('platform_tools/ios/bin/ios_' + script)
    self.m.run(self.m.step,
               name = '%s %s' % (script, first),
               cmd = [full, first] + list(rest),
               infra_step=kInfraStep)

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

  def read_file_on_device(self, path):
    full = self.m.vars.skia_dir.join('platform_tools/ios/bin/ios_cat_file')
    rc = self.m.run(self.m.step,
                    name = 'cat_file %s' % path,
                    cmd = [full, path],
                    stdout=self.m.raw_io.output(),
                    infra_step=kInfraStep)
    return rc.stdout.rstrip() if rc.stdout else rc.stdout
