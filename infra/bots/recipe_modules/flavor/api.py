# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


from recipe_engine import recipe_api

from . import android_flavor
from . import cmake_flavor
from . import coverage_flavor
from . import default_flavor
from . import gn_flavor
from . import ios_flavor
from . import pdfium_flavor
from . import valgrind_flavor
from . import xsan_flavor


def is_android(builder_cfg):
  """Determine whether the given builder is an Android builder."""
  return ('Android' in builder_cfg.get('extra_config', '') or
          builder_cfg.get('os') == 'Android')


def is_cmake(builder_cfg):
  return 'CMake' in builder_cfg.get('extra_config', '')


def is_gn(builder_cfg):
  return 'GN' == builder_cfg.get('extra_config', '')


def is_ios(builder_cfg):
  return ('iOS' in builder_cfg.get('extra_config', '') or
          builder_cfg.get('os') == 'iOS')


def is_pdfium(builder_cfg):
  return 'PDFium' in builder_cfg.get('extra_config', '')


def is_valgrind(builder_cfg):
  return 'Valgrind' in builder_cfg.get('extra_config', '')


def is_xsan(builder_cfg):
  return ('ASAN' in builder_cfg.get('extra_config', '') or
          'MSAN' in builder_cfg.get('extra_config', '') or
          'TSAN' in builder_cfg.get('extra_config', ''))


class SkiaFlavorApi(recipe_api.RecipeApi):
  def get_flavor(self, builder_cfg):
    """Return a flavor utils object specific to the given builder."""
    if is_android(builder_cfg):
      return android_flavor.AndroidFlavorUtils(self.m)
    elif is_cmake(builder_cfg):
      return cmake_flavor.CMakeFlavorUtils(self.m)
    elif is_gn(builder_cfg):
      return gn_flavor.GNFlavorUtils(self.m)
    elif is_ios(builder_cfg):
      return ios_flavor.iOSFlavorUtils(self.m)
    elif is_pdfium(builder_cfg):
      return pdfium_flavor.PDFiumFlavorUtils(self.m)
    elif is_valgrind(builder_cfg):
      return valgrind_flavor.ValgrindFlavorUtils(self.m)
    elif is_xsan(builder_cfg):
      return xsan_flavor.XSanFlavorUtils(self.m)
    elif builder_cfg.get('configuration') == 'Coverage':
      return coverage_flavor.CoverageFlavorUtils(self.m)
    else:
      return default_flavor.DefaultFlavorUtils(self.m)

  def setup(self):
    self._f = self.get_flavor(self.m.vars.builder_cfg)

  def step(self, name, cmd, **kwargs):
    return self._f.step(name, cmd, **kwargs)

  def compile(self, target):
    return self._f.compile(target)

  def copy_extra_build_products(self, swarming_out_dir):
    return self._f.copy_extra_build_products(swarming_out_dir)

  @property
  def out_dir(self):
    return self._f.out_dir

  def device_path_join(self, *args):
    return self._f.device_path_join(*args)

  def device_path_exists(self, path):
    return self._f.device_path_exists(path)  # pragma: no cover

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

  def read_file_on_device(self, path):
    return self._f.read_file_on_device(path)

  def remove_file_on_device(self, path):
    return self._f.remove_file_on_device(path)

  def install(self):
    rv = self._f.install()
    self.device_dirs = self._f.device_dirs
    return rv

  def cleanup_steps(self):
    return self._f.cleanup_steps()
