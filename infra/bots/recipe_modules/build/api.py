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
    self._checkout_root = None
    self._out_dir = None
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

  def set_checkout_root(self, cr):
    self._checkout_root = cr

  def set_out_dir(self, od):
    self._out_dir = od

  @property
  def checkout_root(self):
    if not self._checkout_root:
      self._checkout_root = self.m.core.checkout_root
    return self._checkout_root

  @property
  def skia_dir(self):
    if 'Flutter' in self.m.vars.builder_name:
      return self.checkout_root.join('src', 'third_party', 'skia')
    return self.checkout_root.join('skia')

  @property
  def out_dir(self):
    if not self._out_dir:
      self._out_dir = self.skia_dir.join(
          'out', self.m.vars.builder_name, self.m.vars.configuration)
    return self._out_dir

  def __call__(self):
    """Compile the code."""
    self.compile_fn(self.m, self.skia_dir, self.out_dir)

  def copy_build_products(self, dst):
    """Copy whitelisted build products to dst."""
    util.copy_whitelisted_build_products(self.m, self.out_dir, dst)
    self.copy_fn(self.m, self.skia_dir, self.out_dir, dst)
