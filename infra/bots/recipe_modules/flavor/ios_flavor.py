# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import gn_flavor

class iOSFlavorUtils(gn_flavor.GNFlavorUtils):

  def step(self, name, cmd, env=None, **kwargs):
    app = self.m.vars.skia_out.join(self.m.vars.configuration, cmd[0])

    self._py('package ' + name,
             self.m.vars.skia_dir.join('gn', 'package_ios.py'),
             args=[str(app)])
    self._run(name,
              ['ios-deploy', '-b', '%s.app' % app,
               '-I', '--args', ' '.join(map(str, cmd[1:]))])

  def _ios_bin(self, script):
    return self.m.vars.skia_dir.join('platform_tools', 'ios', 'bin', script)

  def copy_file_to_device(self, host, device):
    self.m.run(self.m.step,
               name = 'push %s' % host,
               cmd = [self._ios_bin('ios_push_file'), host, device],
               infra_step=True)

  def copy_directory_contents_to_device(self, host, device):
    self.m.run(self.m.step,
               name = 'push %s' % host,
               cmd = [self._ios_bin('ios_push_if_needed'), host, device],
               infra_step=True)

  def copy_directory_contents_to_host(self, device, host):
    self.m.run(self.m.step,
               name = 'pull %s' % device,
               cmd = [self._ios_bin('ios_pull_if_needed'), device, host],
               infra_step=True)

  def read_file_on_device(self, path):  # pragma: nocover
    rc = self.m.run(self.m.step,
                    name = 'read %s' % path,
                    cmd = [self._ios_bin('ios_cat_file'), path],
                    stdout=self.m.raw_io.output(),
                    infra_step=True)
    return rc.stdout.rstrip() if rc.stdout else rc.stdout

  def remove_file_on_device(self, path):
    self.m.run(self.m.step,
               name = 'rm %s' % path,
               cmd = [self._ios_bin('ios_rm'), path],
               infra_step=True)


  def create_clean_device_dir(self, path):
    self.remove_file_on_device(path)
    self.m.run(self.m.step,
               name = 'mkdir %s' % path,
               cmd = [self._ios_bin('ios_mkidr'), path],
               infra_step=True)
