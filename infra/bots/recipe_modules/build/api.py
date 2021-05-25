# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Build Skia for various platforms."""


from recipe_engine import recipe_api

from . import android
from . import canvaskit
from . import chromebook
from . import cmake
from . import default
from . import docker
from . import flutter
from . import pathkit


class BuildApi(recipe_api.RecipeApi):
  def __init__(self, buildername, *args, **kwargs):
    b = buildername
    if 'Android' in b and not 'Flutter' in b:
      self.compile_fn = android.compile_fn
      self.copy_fn = android.copy_build_products
    elif 'Chromebook' in b:
      self.compile_fn = chromebook.compile_fn
      self.copy_fn = chromebook.copy_build_products
    elif 'Flutter' in b:
      self.compile_fn = flutter.compile_fn
      self.copy_fn = flutter.copy_build_products
    elif 'EMCC' in b:
      if 'PathKit' in b:
        self.compile_fn = pathkit.compile_fn
        self.copy_fn = pathkit.copy_build_products
      else:
        self.compile_fn = canvaskit.compile_fn
        self.copy_fn = canvaskit.copy_build_products
    elif 'CMake' in b:
      self.compile_fn = cmake.compile_fn
      self.copy_fn = cmake.copy_build_products
    elif 'Docker' in b:
      self.compile_fn = docker.compile_fn
      self.copy_fn = docker.copy_build_products
    else:
      self.compile_fn = default.compile_fn
      self.copy_fn = default.copy_build_products
    super(BuildApi, self).__init__(*args, **kwargs)

  def __call__(self, checkout_root, out_dir):
    """Compile the code."""
    self.compile_fn(self.m, checkout_root, out_dir)

  def copy_build_products(self, out_dir, dst):
    """Copy selected build products to dst."""
    self.copy_fn(self.m, out_dir, dst)
