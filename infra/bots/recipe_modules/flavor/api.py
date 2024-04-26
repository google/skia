# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


from recipe_engine import recipe_api

from . import android
from . import chromebook
from . import default
from . import ios
from . import valgrind


"""Abstractions for running code on various platforms.

The methods in this module define how certain high-level functions should work.
Each flavor should correspond to a subclass of DefaultFlavor which may override
any of these functions as appropriate for that flavor.

For example, the AndroidFlavor will override the functions for copying files
between the host and Android device, as well as the 'step' function, so that
commands may be run through ADB.
"""


VERSION_FILE_LOTTIE = 'LOTTIE_VERSION'
VERSION_FILE_SK_IMAGE = 'SK_IMAGE_VERSION'
VERSION_FILE_SKP = 'SKP_VERSION'
VERSION_FILE_SVG = 'SVG_VERSION'
VERSION_FILE_MSKP = 'MSKP_VERSION'
VERSION_FILE_TEXTTRACES = 'TEXTTRACES_VERSION'

VERSION_NONE = -1

def is_android(vars_api):
  return ('Android' in vars_api.extra_tokens or
          'Android' in vars_api.builder_cfg.get('os', ''))

def is_chromebook(vars_api):
  return ('Chromebook' in vars_api.extra_tokens or
          'ChromeOS' in vars_api.builder_cfg.get('os', ''))

def is_ios(vars_api):
  return ('iOS' in vars_api.extra_tokens or
          'iOS' == vars_api.builder_cfg.get('os', ''))

def is_valgrind(vars_api):
  return 'Valgrind' in vars_api.extra_tokens


class SkiaFlavorApi(recipe_api.RecipeApi):
  def get_flavor(self, vars_api, app_name):
    """Return a flavor utils object specific to the given builder."""
    if is_chromebook(vars_api):
      return chromebook.ChromebookFlavor(self, app_name)
    if is_android(vars_api):
      return android.AndroidFlavor(self, app_name)
    elif is_ios(vars_api):
      return ios.iOSFlavor(self, app_name)
    elif is_valgrind(vars_api):
      return valgrind.ValgrindFlavor(self, app_name)
    else:
      return default.DefaultFlavor(self, app_name)

  def setup(self, app_name):
    self._f = self.get_flavor(self.m.vars, app_name)
    self.device_dirs = self._f.device_dirs
    self.host_dirs = self._f.host_dirs
    self._skia_dir = self.m.path.start_dir.join('skia')

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
              resources=False, mskps=False, texttraces=False):
    self._f.install()

    if texttraces:
      self._copy_texttraces()
    # TODO(borenet): Only copy files which have changed.
    if resources:
      self.copy_directory_contents_to_device(
          self.m.path.start_dir.join('skia', 'resources'),
          self.device_dirs.resource_dir)

    if skps:
      self._copy_skps()
    if images:
      self._copy_images()
    if lotties:
      self._copy_lotties()
    if svgs:
      self._copy_svgs()
    if mskps:
      self._copy_mskps()

  def cleanup_steps(self):
    return self._f.cleanup_steps()

  def _copy_dir(self, host_version, version_file, tmp_dir,
                host_path, device_path):
    actual_version_file = self.m.path.join(tmp_dir, version_file)
    # Copy to device.
    device_version_file = self.device_path_join(
        self.device_dirs.tmp_dir, version_file)
    if str(actual_version_file) != str(device_version_file):
      device_version = self.read_file_on_device(device_version_file,
                                                abort_on_failure=False,
                                                fail_build_on_failure=False)
      if not device_version:
        device_version = VERSION_NONE
      if device_version != host_version:
        self.remove_file_on_device(device_version_file)
        self.create_clean_device_dir(device_path)
        self.copy_directory_contents_to_device(
            host_path, device_path)

        # Copy the new version file.
        self.copy_file_to_device(actual_version_file, device_version_file)

  def _copy_images(self):
    """Copy test images if needed."""
    version = self.m.run.asset_version('skimage', self._skia_dir)
    self.m.run.writefile(
        self.m.path.join(self.m.vars.tmp_dir, VERSION_FILE_SK_IMAGE),
        version)
    self._copy_dir(
        version,
        VERSION_FILE_SK_IMAGE,
        self.m.vars.tmp_dir,
        self.host_dirs.images_dir,
        self.device_dirs.images_dir)
    return version

  def _copy_lotties(self):
    """Copy test lotties if needed."""
    version = self.m.run.asset_version('lottie-samples', self._skia_dir)
    self.m.run.writefile(
        self.m.path.join(self.m.vars.tmp_dir, VERSION_FILE_LOTTIE),
        version)
    self._copy_dir(
        version,
        VERSION_FILE_LOTTIE,
        self.m.vars.tmp_dir,
        self.host_dirs.lotties_dir,
        self.device_dirs.lotties_dir)
    return version

  def _copy_skps(self):
    """Copy the SKPs if needed."""
    version = self.m.run.asset_version('skp', self._skia_dir)
    self.m.run.writefile(
        self.m.path.join(self.m.vars.tmp_dir, VERSION_FILE_SKP),
        version)
    self._copy_dir(
        version,
        VERSION_FILE_SKP,
        self.m.vars.tmp_dir,
        self.host_dirs.skp_dir,
        self.device_dirs.skp_dir)
    return version

  def _copy_svgs(self):
    """Copy the SVGs if needed."""
    version = self.m.run.asset_version('svg', self._skia_dir)
    self.m.run.writefile(
        self.m.path.join(self.m.vars.tmp_dir, VERSION_FILE_SVG),
        version)
    self._copy_dir(
        version,
        VERSION_FILE_SVG,
        self.m.vars.tmp_dir,
        self.host_dirs.svg_dir,
        self.device_dirs.svg_dir)
    return version

  def _copy_mskps(self):
    """Copy the MSKPs if needed."""
    version = self.m.run.asset_version('mskp', self._skia_dir)
    self.m.run.writefile(
        self.m.path.join(self.m.vars.tmp_dir, VERSION_FILE_MSKP),
        version)
    self._copy_dir(
        version,
        VERSION_FILE_MSKP,
        self.m.vars.tmp_dir,
        self.host_dirs.mskp_dir,
        self.device_dirs.mskp_dir)
    return version

  def _copy_texttraces(self):
    """Copy the text traces if needed."""
    version = self.m.run.asset_version('text_blob_traces', self._skia_dir)
    self.m.run.writefile(
        self.m.path.join(self.m.vars.tmp_dir, VERSION_FILE_TEXTTRACES),
        version)
    self._copy_dir(
        version,
        VERSION_FILE_TEXTTRACES,
        self.m.vars.tmp_dir,
        self.host_dirs.texttraces_dir,
        self.device_dirs.texttraces_dir)
    return version
