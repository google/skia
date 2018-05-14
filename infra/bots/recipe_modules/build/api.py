# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Build Skia for various platforms."""


from recipe_engine import recipe_api

from . import android
from . import chromebook
from . import chromecast
from . import default
from . import flutter
from . import util


class BuildApi(recipe_api.RecipeApi):
  def __init__(self, buildername, *args, **kwargs):
    b = buildername
    if 'Android' in b and not 'Flutter' in b:
      self.compile_fn = android.compile_fn
      self.copy_fn = android.copy_extra_build_products
    elif 'Chromebook' in b:
      self.compile_fn = chromebook.compile_fn
      self.copy_fn = chromebook.copy_extra_build_products
    elif 'Chromecast' in b:
      self.compile_fn = chromecast.compile_fn
      self.copy_fn = chromecast.copy_extra_build_products
    elif 'Flutter' in b:
      self.compile_fn = flutter.compile_fn
      self.copy_fn = flutter.copy_extra_build_products
    else:
      self.compile_fn = default.compile_fn
      self.copy_fn = default.copy_extra_build_products
    super(BuildApi, self).__init__(*args, **kwargs)

  def __call__(self):
    """Compile the code."""
    out_dir = self.m.vars.skia_out.join(self.m.vars.configuration)
    self.compile_fn(self.m, out_dir)

  def copy_build_products(self, dst):
    """Copy whitelisted build products to dst."""
    src = self.m.vars.skia_out.join(self.m.vars.configuration)
    util.copy_whitelisted_build_products(self.m, src, dst)
    self.copy_fn(self.m, src, dst)
