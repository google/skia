# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api


class GitApi(recipe_api.RecipeApi):
  def env(self):
    """Add Git to PATH

    Requires the infra/git and infra/tools/git CIPD packages to be installed
    in the 'git' relative path.
    """
    git_dir = self.m.path.start_dir.join('git')
    git_bin = git_dir.join('bin')
    return self.m.env({'PATH': self.m.path.pathsep.join(
        [str(git_dir), str(git_bin), '%(PATH)s'])})
