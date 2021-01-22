# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api


INFRA_GO_PKG = 'go.skia.org/infra'
UPDATE_GO_ATTEMPTS = 5
UPLOAD_ATTEMPTS = 5


class InfraApi(recipe_api.RecipeApi):
  @property
  def goroot(self):
    return self.m.vars.workdir.join('go', 'go')

  @property
  def go_bin(self):
    return self.goroot.join('bin')

  @property
  def go_env(self):
    return {
        'GOCACHE': self.m.vars.cache_dir.join('go_cache'),
        'GOPATH': self.gopath,
        'GOROOT': self.goroot,
        'PATH': self.m.path.pathsep.join([
            str(self.go_bin), str(self.gopath.join('bin')), '%(PATH)s']),
    }

  @property
  def gopath(self):
    return self.m.vars.cache_dir.join('gopath')
