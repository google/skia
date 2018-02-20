# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


"""Default flavor utils class, used for desktop builders."""


import json


WIN_TOOLCHAIN_DIR = 't'


class DeviceDirs(object):
  def __init__(self,
               dm_dir,
               perf_data_dir,
               resource_dir,
               images_dir,
               skp_dir,
               svg_dir,
               tmp_dir):
    self._dm_dir = dm_dir
    self._perf_data_dir = perf_data_dir
    self._resource_dir = resource_dir
    self._images_dir = images_dir
    self._skp_dir = skp_dir
    self._svg_dir = svg_dir
    self._tmp_dir = tmp_dir

  @property
  def dm_dir(self):
    """Where DM writes."""
    return self._dm_dir

  @property
  def perf_data_dir(self):
    return self._perf_data_dir

  @property
  def resource_dir(self):
    return self._resource_dir

  @property
  def images_dir(self):
    return self._images_dir

  @property
  def skp_dir(self):
    """Holds SKP files that are consumed by RenderSKPs and BenchPictures."""
    return self._skp_dir

  @property
  def svg_dir(self):
    return self._svg_dir

  @property
  def tmp_dir(self):
    return self._tmp_dir


class DefaultFlavorUtils(object):
  """Utilities to be used by build steps.

  The methods in this class define how certain high-level functions should
  work. Each build step flavor should correspond to a subclass of
  DefaultFlavorUtils which may override any of these functions as appropriate
  for that flavor.

  For example, the AndroidFlavorUtils will override the functions for
  copying files between the host and Android device, as well as the
  'step' function, so that commands may be run through ADB.
  """
  def __init__(self, module):
    # Store a pointer to the parent recipe module (SkiaFlavorApi) so that
    # FlavorUtils objects can do recipe module-like things, like run steps or
    # access module-level resources.
    self.module = module

    # self.m is just a shortcut so that FlavorUtils objects can use the same
    # syntax as regular recipe modules to run steps, eg: self.m.step(...)
    self.m = module.m
    self._chrome_path = None
    self._win_toolchain_dir = self.m.vars.slave_dir.join(WIN_TOOLCHAIN_DIR)
    win_toolchain_asset_path = self.m.vars.infrabots_dir.join(
        'assets', 'win_toolchain', 'VERSION')
    if not self.m.path.exists(win_toolchain_asset_path):
      self._win_toolchain_dir = self.m.vars.slave_dir

  def copy_extra_build_products(self, swarming_out_dir):
    pass

  @property
  def out_dir(self):
    """Flavor-specific out directory."""
    return self.m.vars.skia_out.join(self.m.vars.configuration)

  def device_path_join(self, *args):
    """Like os.path.join(), but for paths on a connected device."""
    return self.m.path.join(*args)

  def copy_directory_contents_to_device(self, host_dir, device_dir):
    """Like shutil.copytree(), but for copying to a connected device."""
    # For "normal" builders who don't have an attached device, we expect
    # host_dir and device_dir to be the same.
    if str(host_dir) != str(device_dir):
      raise ValueError('For builders who do not have attached devices, copying '
                       'from host to device is undefined and only allowed if '
                       'host_path and device_path are the same (%s vs %s).' % (
                       str(host_dir), str(device_dir)))

  def copy_directory_contents_to_host(self, device_dir, host_dir):
    """Like shutil.copytree(), but for copying from a connected device."""
    # For "normal" builders who don't have an attached device, we expect
    # host_dir and device_dir to be the same.
    if str(host_dir) != str(device_dir):
      raise ValueError('For builders who do not have attached devices, copying '
                       'from device to host is undefined and only allowed if '
                       'host_path and device_path are the same (%s vs %s).' % (
                       str(host_dir), str(device_dir)))

  def copy_file_to_device(self, host_path, device_path):
    """Like shutil.copyfile, but for copying to a connected device."""
    # For "normal" builders who don't have an attached device, we expect
    # host_dir and device_dir to be the same.
    if str(host_path) != str(device_path):
      raise ValueError('For builders who do not have attached devices, copying '
                       'from host to device is undefined and only allowed if '
                       'host_path and device_path are the same (%s vs %s).' % (
                       str(host_path), str(device_path)))

  def create_clean_device_dir(self, path):
    """Like shutil.rmtree() + os.makedirs(), but on a connected device."""
    self.create_clean_host_dir(path)

  def create_clean_host_dir(self, path):
    """Convenience function for creating a clean directory."""
    self.m.run.rmtree(path)
    self.m.file.ensure_directory(
        'makedirs %s' % self.m.path.basename(path), path)

  def install(self):
    """Run device-specific installation steps."""
    self.device_dirs = DeviceDirs(
        dm_dir=self.m.vars.dm_dir,
        perf_data_dir=self.m.vars.perf_data_dir,
        resource_dir=self.m.vars.resource_dir,
        images_dir=self.m.vars.images_dir,
        skp_dir=self.m.vars.local_skp_dir,
        svg_dir=self.m.vars.local_svg_dir,
        tmp_dir=self.m.vars.tmp_dir)

  def cleanup_steps(self):
    """Run any device-specific cleanup steps."""
    pass

  def get_branch(self):
    """Returns the branch of the current tryjob run by querying Gerrit."""
    # get the issue and patchset ids.
    issue = str(self.m.properties.get('patch_issue', ''))
    patchset = str(self.m.properties.get('patch_set', ''))

    # if none are provided we assume master.
    if not issue or not patchset:
      return "master"

    # inline python script called below.
    script="""
import contextlib
import json
import math
import socket
import sys
import time
import urllib2

GERRIT_URL_TMPL = 'https://skia-review.googlesource.com/changes/%s/detail'
RETRIES = 3
TIMEOUT = 10
WAIT_BASE = 10

socket.setdefaulttimeout(TIMEOUT)
issue_id = int(sys.argv[1])
for retry in range(RETRIES):
  try:
    url = GERRIT_URL_TMPL % issue_id
    with contextlib.closing(urllib2.urlopen(url, timeout=TIMEOUT)) as w:
      # strip out the XSS prefix.
      body = w.read().lstrip().lstrip(")]}'")
      print json.loads(body)['branch']
      break
  except Exception as e:
    print 'Failed to get branch for issue %s:' % issue_id
    print e
    if retry == RETRIES:
      raise
    waittime = WAIT_BASE * math.pow(2, retry)
    print 'Retry in %d seconds.' % waittime
    time.sleep(waittime)
"""

    # Make a call to Gerrit to retrieve the branch.
    branch = self.m.run(
      self.m.python.inline,
      'get branch for issue',
      program=script,
      args=[issue],
      abort_on_failure=True,
      fail_build_on_failure=True,
      infra_step=True,
      stdout=self.m.raw_io.output()).stdout
    # Use the last line of the output since it will contain the branch name.
    branch = [x.strip() for x in branch.splitlines()][-1]
    return branch or 'master'
