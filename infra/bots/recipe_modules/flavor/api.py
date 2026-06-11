# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


from recipe_engine import recipe_api

from . import android
from . import chromebook
from . import default
from . import ios


"""Abstractions for running code on various platforms.

The methods in this module define how certain high-level functions should work.
Each flavor should correspond to a subclass of DefaultFlavor which may override
any of these functions as appropriate for that flavor.

For example, the AndroidFlavor will override the functions for copying files
between the host and Android device, as well as the 'step' function, so that
commands may be run through ADB.
"""


def is_android(vars_api):
  return ('Android' in vars_api.extra_tokens or
          'Android' in vars_api.builder_cfg.get('os', ''))

def is_chromebook(vars_api):
  return ('Chromebook' in vars_api.extra_tokens or
          'ChromeOS' in vars_api.builder_cfg.get('os', ''))

def is_ios(vars_api):
  return ('iOS' in vars_api.extra_tokens or
          'iOS' in vars_api.builder_cfg.get('os', ''))

class SkiaFlavorApi(recipe_api.RecipeApi):
  def get_flavor(self, vars_api, app_name):
    """Return a flavor utils object specific to the given builder."""
    if is_chromebook(vars_api):
      return chromebook.ChromebookFlavor(self, app_name)
    if is_android(vars_api):
      return android.AndroidFlavor(self, app_name)
    elif is_ios(vars_api):
      return ios.iOSFlavor(self, app_name)
    else:
      return default.DefaultFlavor(self, app_name)

  def setup(self, app_name):
    self._f = self.get_flavor(self.m.vars, app_name)
    self.device_dirs = self._f.device_dirs
    self.host_dirs = self._f.host_dirs
    self._skia_dir = self.m.path.start_dir.joinpath('skia')

  def step(self, name, cmd, **kwargs):
    return self._f.step(name, cmd, **kwargs)

  def device_path_join(self, *args):
    return self._f.device_path_join(*args)

  def copy_directory_contents_to_device(self, host_dir, device_dir):
    return self._f.copy_directory_contents_to_device(host_dir, device_dir)

  def copy_directory_contents_to_host(self, device_dir, host_dir):
    return self._f.copy_directory_contents_to_host(device_dir, host_dir)

  def copy_file_to_device(self, host_path, device_path):
    return self._f.copy_file_to_device(host_path, device_path)

  def create_clean_host_dir(self, path):
    return self._f.create_clean_host_dir(path)

  def create_clean_device_dir(self, path):
    return self._f.create_clean_device_dir(path)

  def read_file_on_device(self, path, **kwargs):
    return self._f.read_file_on_device(path, **kwargs)

  def remove_file_on_device(self, path):
    return self._f.remove_file_on_device(path)

  def install(self, skps=False, images=False, lotties=False, svgs=False,
              resources=False, texttraces=False):
    self._f.install()

    if texttraces:
      self.copy_directory_contents_to_device(
          self.host_dirs.texttraces_dir,
          self.device_dirs.texttraces_dir)

    if resources:
      self.copy_directory_contents_to_device(
          self.m.path.start_dir.joinpath('skia', 'resources'),
          self.device_dirs.resource_dir)

    if skps:
      self.copy_directory_contents_to_device(
          self.host_dirs.skp_dir,
          self.device_dirs.skp_dir)

    if images:
      self.copy_directory_contents_to_device(
          self.host_dirs.images_dir,
          self.device_dirs.images_dir)

    if lotties:
      self.copy_directory_contents_to_device(
          self.host_dirs.lotties_dir,
          self.device_dirs.lotties_dir)

    if svgs:
      self.copy_directory_contents_to_device(
          self.host_dirs.svg_dir,
          self.device_dirs.svg_dir)

  def cleanup_steps(self):
    return self._f.cleanup_steps()
