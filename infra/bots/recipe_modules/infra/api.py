# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api


INFRA_GO_PKG = 'go.skia.org/infra'
UPDATE_GO_ATTEMPTS = 5


class InfraApi(recipe_api.RecipeApi):
  @property
  def goroot(self):
    return self.m.vars.slave_dir.join('go', 'go')

  @property
  def go_bin(self):
    return self.goroot.join('bin')

  @property
  def go_exe(self):
    return self.go_bin.join('go')

  @property
  def go_env(self):
    return {
        'GOPATH': self.gopath,
        'GOROOT': self.goroot,
        'PATH': '%s:%s:%%(PATH)s' % (self.go_bin, self.gopath),
    }

  @property
  def gopath(self):
    return self.m.vars.slave_dir.join('gopath')

  def go_version(self):
    """Print the Go version."""
    env = self.m.step.get_from_context('env', {})
    env.update(self.go_env)
    with self.m.context(env=env):
      self.m.run(
          self.m.step,
          'go version',
          cmd=[self.go_exe, 'version'])
      self.m.run(
          self.m.step,
          'env go version',
          cmd=['go', 'version'])

  def update_go_deps(self):
    """Attempt to update go dependencies.

    This fails flakily sometimes, so perform multiple attempts.
    """
    self.go_version()
    env = self.m.step.get_from_context('env', {})
    env.update(self.go_env)
    with self.m.context(env=env):
      self.m.run.with_retry(
          self.m.step,
          'update go pkgs',
          UPDATE_GO_ATTEMPTS,
          cmd=[self.go_exe, 'get', '-u', '-t', '%s/...' % INFRA_GO_PKG])
