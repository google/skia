#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Utilities for managing assets."""


import argparse
import os
import shlex
import shutil
import subprocess
import sys

SKIA_DIR = os.path.abspath(os.path.realpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    os.pardir, os.pardir, os.pardir)))
INFRA_BOTS_DIR = os.path.join(SKIA_DIR, 'infra', 'bots')
sys.path.insert(0, INFRA_BOTS_DIR)
import utils
import zip_utils


ASSETS_DIR = os.path.join(INFRA_BOTS_DIR, 'assets')
DEFAULT_GS_BUCKET = 'skia-buildbots'
GS_SUBDIR_TMPL = 'gs://%s/assets/%s'
GS_PATH_TMPL = '%s/%s.zip'
VERSION_FILENAME = 'VERSION'
ZIP_BLACKLIST = ['.git', '.svn', '*.pyc', '.DS_STORE']


class _GSWrapper(object):
  """Wrapper object for interacting with Google Storage."""
  def __init__(self, gsutil):
    gsutil = os.path.abspath(gsutil) if gsutil else 'gsutil'
    self._gsutil = [gsutil]
    if gsutil.endswith('.py'):
      self._gsutil = ['python', gsutil]

  def copy(self, src, dst):
    """Copy src to dst."""
    subprocess.check_call(self._gsutil + ['cp', src, dst])

  def list(self, path):
    """List objects in the given path."""
    try:
      return subprocess.check_output(self._gsutil + ['ls', path]).splitlines()
    except subprocess.CalledProcessError:
      # If the prefix does not exist, we'll get an error, which is okay.
      return []


def _prompt(prompt):
  """Prompt for input, return result."""
  return raw_input(prompt)


class Asset(object):
  def __init__(self, name, gs_bucket=DEFAULT_GS_BUCKET, gsutil=None):
    self._gs = _GSWrapper(gsutil)
    self._gs_subdir = GS_SUBDIR_TMPL % (gs_bucket, name)
    self._name = name
    self._dir = os.path.join(ASSETS_DIR, self._name)

  @property
  def version_file(self):
    """Return the path to the version file for this asset."""
    return os.path.join(self._dir, VERSION_FILENAME)

  def get_current_version(self):
    """Obtain the current version of the asset."""
    if not os.path.isfile(self.version_file):
      return -1
    with open(self.version_file) as f:
      return int(f.read())

  def get_available_versions(self):
    """Return the existing version numbers for this asset."""
    files = self._gs.list(self._gs_subdir)
    bnames = [os.path.basename(f) for f in files]
    suffix = '.zip'
    versions = [int(f[:-len(suffix)]) for f in bnames if f.endswith(suffix)]
    versions.sort()
    return versions

  def get_next_version(self):
    """Find the next available version number for the asset."""
    versions = self.get_available_versions()
    if len(versions) == 0:
      return 0
    return versions[-1] + 1

  def download_version(self, version, target_dir):
    """Download the specified version of the asset."""
    gs_path = GS_PATH_TMPL % (self._gs_subdir, str(version))
    target_dir = os.path.abspath(target_dir)
    with utils.tmp_dir():
      zip_file = os.path.join(os.getcwd(), '%d.zip' % version)
      self._gs.copy(gs_path, zip_file)
      zip_utils.unzip(zip_file, target_dir)

  def download_current_version(self, target_dir):
    """Download the version of the asset specified in its version file."""
    v = self.get_current_version()
    self.download_version(v, target_dir)

  def upload_new_version(self, target_dir, commit=False):
    """Upload a new version and update the version file for the asset."""
    version = self.get_next_version()
    target_dir = os.path.abspath(target_dir)
    with utils.tmp_dir():
      zip_file = os.path.join(os.getcwd(), '%d.zip' % version)
      zip_utils.zip(target_dir, zip_file, blacklist=ZIP_BLACKLIST)
      gs_path = GS_PATH_TMPL % (self._gs_subdir, str(version))
      self._gs.copy(zip_file, gs_path)

    def _write_version():
      with open(self.version_file, 'w') as f:
        f.write(str(version))
      subprocess.check_call([utils.GIT, 'add', self.version_file])

    with utils.chdir(SKIA_DIR):
      if commit:
        with utils.git_branch():
          _write_version()
          subprocess.check_call([
              utils.GIT, 'commit', '-m', 'Update %s version' % self._name])
          subprocess.check_call([utils.GIT, 'cl', 'upload', '--bypass-hooks'])
      else:
        _write_version()

  @classmethod
  def add(cls, name, gs_bucket=DEFAULT_GS_BUCKET, gsutil=None):
    """Add an asset."""
    asset = cls(name, gs_bucket=gs_bucket, gsutil=gsutil)
    if os.path.isdir(asset._dir):
      raise Exception('Asset %s already exists!' % asset._name)

    print 'Creating asset in %s' % asset._dir
    os.mkdir(asset._dir)
    def copy_script(script):
      src = os.path.join(ASSETS_DIR, 'scripts', script)
      dst = os.path.join(asset._dir, script)
      print 'Creating %s' % dst
      shutil.copy(src, dst)
      subprocess.check_call([utils.GIT, 'add', dst])

    for script in ('download.py', 'upload.py', 'common.py'):
      copy_script(script)
    resp = _prompt('Add script to automate creation of this asset? (y/n) ')
    if resp == 'y':
      copy_script('create.py')
      copy_script('create_and_upload.py')
      print 'You will need to add implementation to the creation script.'
    print 'Successfully created asset %s.' % asset._name
    return asset

  def remove(self):
    """Remove this asset."""
    # Ensure that the asset exists.
    if not os.path.isdir(self._dir):
      raise Exception('Asset %s does not exist!' % self._name)

    # Remove the asset.
    subprocess.check_call([utils.GIT, 'rm', '-rf', self._dir])
    if os.path.isdir(self._dir):
      shutil.rmtree(self._dir)

    # We *could* remove all uploaded versions of the asset in Google Storage but
    # we choose not to be that destructive.
