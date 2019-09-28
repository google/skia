# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Build Skia for various platforms."""


from recipe_engine import recipe_api

from . import android
from . import canvaskit
from . import chromebook
from . import chromecast
from . import cmake
from . import default
from . import flutter
from . import pathkit
from . import skqp
from . import util


class BuildApi(recipe_api.RecipeApi):
  def __init__(self, buildername, *args, **kwargs):
    b = buildername
    if 'SKQP' in b and not 'Test' in b:
      self.compile_fn = skqp.compile_fn
      self.copy_fn = skqp.copy_extra_build_products
    elif 'Android' in b and not 'Flutter' in b:
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
    elif 'EMCC' in b:
      if 'PathKit' in b:
        self.compile_fn = pathkit.compile_fn
        self.copy_fn = pathkit.copy_extra_build_products
      else:
        self.compile_fn = canvaskit.compile_fn
        self.copy_fn = canvaskit.copy_extra_build_products
    elif 'CMake' in b:
      self.compile_fn = cmake.compile_fn
      self.copy_fn = cmake.copy_extra_build_products
    else:
      self.compile_fn = default.compile_fn
      self.copy_fn = default.copy_extra_build_products
    super(BuildApi, self).__init__(*args, **kwargs)

  def __call__(self, checkout_root, out_dir):
    """Compile the code."""
    self.compile_fn(self.m, checkout_root, out_dir)

  def copy_build_products(self, out_dir, dst):
    """Copy whitelisted build products to dst."""
    util.copy_whitelisted_build_products(self.m, out_dir, dst)
    self.copy_fn(self.m, out_dir, dst)
