# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


from recipe_engine import recipe_api

from . import default_flavor
from . import flutter_flavor
from . import gn_android_flavor
from . import gn_chromebook_flavor
from . import gn_chromecast_flavor
from . import gn_flavor
from . import ios_flavor
from . import pdfium_flavor
from . import valgrind_flavor


TEST_EXPECTED_SKP_VERSION = '42'
TEST_EXPECTED_SVG_VERSION = '42'
TEST_EXPECTED_SK_IMAGE_VERSION = '42'

VERSION_FILE_SK_IMAGE = 'SK_IMAGE_VERSION'
VERSION_FILE_SKP = 'SKP_VERSION'
VERSION_FILE_SVG = 'SVG_VERSION'

VERSION_NONE = -1

def is_android(builder_cfg):
  return 'Android' in builder_cfg.get('extra_config', '')

def is_chromecast(builder_cfg):
  return ('Chromecast' in builder_cfg.get('extra_config', '') or
          'Chromecast' in builder_cfg.get('os', ''))

def is_chromebook(builder_cfg):
  return ('Chromebook' in builder_cfg.get('extra_config', '') or
          'ChromeOS' in builder_cfg.get('os', ''))

def is_flutter(builder_cfg):
  return 'Flutter' in builder_cfg.get('extra_config', '')

def is_ios(builder_cfg):
  return ('iOS' in builder_cfg.get('extra_config', '') or
          'iOS' == builder_cfg.get('os', ''))

def is_pdfium(builder_cfg):
  return 'PDFium' in builder_cfg.get('extra_config', '')

def is_valgrind(builder_cfg):
  return 'Valgrind' in builder_cfg.get('extra_config', '')


class SkiaFlavorApi(recipe_api.RecipeApi):
  def get_flavor(self, builder_cfg):
    """Return a flavor utils object specific to the given builder."""
    if is_flutter(builder_cfg):
      return flutter_flavor.FlutterFlavorUtils(self)
    if is_chromecast(builder_cfg):
      return gn_chromecast_flavor.GNChromecastFlavorUtils(self)
    if is_chromebook(builder_cfg):
      return gn_chromebook_flavor.GNChromebookFlavorUtils(self)
    if is_android(builder_cfg):
      return gn_android_flavor.GNAndroidFlavorUtils(self)
    elif is_ios(builder_cfg):
      return ios_flavor.iOSFlavorUtils(self)
    elif is_pdfium(builder_cfg):
      return pdfium_flavor.PDFiumFlavorUtils(self)
    elif is_valgrind(builder_cfg):
      return valgrind_flavor.ValgrindFlavorUtils(self)
    else:
      return gn_flavor.GNFlavorUtils(self)

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

  def install_everything(self):
    self.install(skps=True, images=True, svgs=True, resources=True)

  def install(self, skps=False, images=False, svgs=False, resources=False):
    self._f.install()
    self.device_dirs = self._f.device_dirs

    # TODO(borenet): Only copy files which have changed.
    if resources:
      self.copy_directory_contents_to_device(
          self.m.vars.resource_dir,
          self.device_dirs.resource_dir)

    if skps:
      self._copy_skps()
    if images:
      self._copy_images()
    if svgs:
      self._copy_svgs()

  def cleanup_steps(self):
    return self._f.cleanup_steps()

  def _copy_dir(self, host_version, version_file, tmp_dir,
                host_path, device_path, test_expected_version,
                test_actual_version):
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
    """Download and copy test images if needed."""
    version_file = self.m.vars.infrabots_dir.join(
        'assets', 'skimage', 'VERSION')
    test_data = self.m.properties.get(
        'test_downloaded_sk_image_version', TEST_EXPECTED_SK_IMAGE_VERSION)
    version = self.m.run.readfile(
        version_file,
        name='Get downloaded skimage VERSION',
        test_data=test_data).rstrip()
    self.m.run.writefile(
        self.m.path.join(self.m.vars.tmp_dir, VERSION_FILE_SK_IMAGE),
        version)
    self._copy_dir(
        version,
        VERSION_FILE_SK_IMAGE,
        self.m.vars.tmp_dir,
        self.m.vars.images_dir,
        self.device_dirs.images_dir,
        test_expected_version=self.m.properties.get(
            'test_downloaded_sk_image_version',
            TEST_EXPECTED_SK_IMAGE_VERSION),
        test_actual_version=self.m.properties.get(
            'test_downloaded_sk_image_version',
            TEST_EXPECTED_SK_IMAGE_VERSION))
    return version

  def _copy_skps(self):
    """Download and copy the SKPs if needed."""
    version_file = self.m.vars.infrabots_dir.join(
        'assets', 'skp', 'VERSION')
    test_data = self.m.properties.get(
        'test_downloaded_skp_version', TEST_EXPECTED_SKP_VERSION)
    version = self.m.run.readfile(
        version_file,
        name='Get downloaded SKP VERSION',
        test_data=test_data).rstrip()
    self.m.run.writefile(
        self.m.path.join(self.m.vars.tmp_dir, VERSION_FILE_SKP),
        version)
    self._copy_dir(
        version,
        VERSION_FILE_SKP,
        self.m.vars.tmp_dir,
        self.m.vars.local_skp_dir,
        self.device_dirs.skp_dir,
        test_expected_version=self.m.properties.get(
            'test_downloaded_skp_version', TEST_EXPECTED_SKP_VERSION),
        test_actual_version=self.m.properties.get(
            'test_downloaded_skp_version', TEST_EXPECTED_SKP_VERSION))
    return version

  def _copy_svgs(self):
    """Download and copy the SVGs if needed."""
    version_file = self.m.vars.infrabots_dir.join(
        'assets', 'svg', 'VERSION')
    test_data = self.m.properties.get(
        'test_downloaded_svg_version', TEST_EXPECTED_SVG_VERSION)
    version = self.m.run.readfile(
        version_file,
        name='Get downloaded SVG VERSION',
        test_data=test_data).rstrip()
    self.m.run.writefile(
        self.m.path.join(self.m.vars.tmp_dir, VERSION_FILE_SVG),
        version)
    self._copy_dir(
        version,
        VERSION_FILE_SVG,
        self.m.vars.tmp_dir,
        self.m.vars.local_svg_dir,
        self.device_dirs.svg_dir,
        test_expected_version=self.m.properties.get(
            'test_downloaded_svg_version', TEST_EXPECTED_SVG_VERSION),
        test_actual_version=self.m.properties.get(
            'test_downloaded_svg_version', TEST_EXPECTED_SVG_VERSION))
    return version
