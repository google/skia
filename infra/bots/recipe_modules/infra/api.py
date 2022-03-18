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
    go_root = self.m.vars.workdir.join('go', 'go')
    # Starting with Go 1.18, the standard library includes "//go:embed"
    # directives that point to other files in the standard library. For
    # security reasons, the "embed" package does not support symbolic links
    # (discussion at
    # https://github.com/golang/go/issues/35950#issuecomment-561725322), and it
    # produces "cannot embed irregular file" errors when it encounters one.
    #
    # To prevent the above error, we ensure our GOROOT environment variable
    # points to a path without symbolic links.
    #
    # For some reason step.m.path.realpath returns the path unchanged, so we
    # invoke realpath instead.
    symlink_version_file = go_root.join('VERSION') # Arbitrary symlink.
    step_result = self.m.step('realpath go/go/VERSION',
                          cmd=['realpath', str(symlink_version_file)],
                          stdout=self.m.raw_io.output_text())
    step_result = self.m.step('dirname',
                          cmd=['dirname', step_result.stdout],
                          stdout=self.m.raw_io.output_text())
    go_root_nosymlinks = step_result.stdout.strip()
    if go_root_nosymlinks != "":
      return go_root_nosymlinks # pragma: nocover
    else:
      # This branch exists solely to appease recipe tests, under which the
      # workdir variable is unset. Returning an empty string causes tests to
      # fail, so we return the original GOROOT instead.
      return go_root

  @property
  def go_bin(self):
    return self.m.path.join(self.goroot, 'bin')

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
