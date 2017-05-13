# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api


class EnvApi(recipe_api.RecipeApi):
  def __call__(self, env_dict):
    env = self.m.context.env
    # If PATH is defined in both, merge them together, merging default_env into
    # path by replacing %(PATH)s
    upstream_path = env.get('PATH', '')
    env.update(env_dict)
    my_path = env_dict.get('PATH', '')
    if upstream_path and my_path and upstream_path != my_path:
      env['PATH'] = upstream_path.replace(r'%(PATH)s', my_path)

    return self.m.context(env=env)
