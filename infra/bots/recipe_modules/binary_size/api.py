# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api
from recipe_engine import config_types


class BinarySizeApi(recipe_api.RecipeApi):
  def run_analysis(self, skia_dir, dest_file):
    cmd = ['python', self.resource('run_binary_size_analysis.py'),
           '--library', self.m.vars.build_dir.join('libskia.so'),
           '--githash', self.m.properties['revision'],
           '--dest', dest_file]
    if self.m.vars.is_trybot:
      cmd.extend(['--issue_number', str(self.m.properties['patch_issue'])])
    with self.m.context(cwd=skia_dir):
      self.m.run(
        self.m.step,
        'generate binary size data',
        cmd=cmd)
