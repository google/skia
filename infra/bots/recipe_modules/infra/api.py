# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api


INFRA_GO_PKG = 'go.skia.org/infra'
UPDATE_GO_ATTEMPTS = 5


class InfraApi(recipe_api.RecipeApi):
  @property
  def go_env(self):
    return {'GOPATH': self.gopath}

  @property
  def gopath(self):
    return self.m.vars.checkout_root.join('gopath')

  def update_go_deps(self):
    """Attempt to update go dependencies.

    This fails flakily sometimes, so perform multiple attempts.
    """
    env = self.m.step.get_from_context('env', {})
    env.update(self.go_env)
    with self.m.step.context({'env': env}):
      self.m.run.with_retry(
          self.m.step,
          'update go pkgs',
          UPDATE_GO_ATTEMPTS,
          cmd=['go', 'get', '-u', '-t', '%s/...' % INFRA_GO_PKG])
