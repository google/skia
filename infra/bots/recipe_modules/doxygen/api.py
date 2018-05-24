# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api
from recipe_engine import config_types


class DoxygenApi(recipe_api.RecipeApi):
  def generate_and_upload(self, skia_dir):
    with self.m.context(cwd=skia_dir):
      self.m.run(
          self.m.step,
          'generate and upload doxygen',
          cmd=['python', self.resource('generate_and_upload_doxygen.py')],
          abort_on_failure=False)
