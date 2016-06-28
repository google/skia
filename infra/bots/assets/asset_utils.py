#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Utilities for managing assets."""


import argparse
import json
import os
import shlex
import shutil
import subprocess
import sys

INFRA_BOTS_DIR = os.path.abspath(os.path.realpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), os.pardir)))
sys.path.insert(0, INFRA_BOTS_DIR)
import utils
import zip_utils


ASSETS_DIR = os.path.join(INFRA_BOTS_DIR, 'assets')
SKIA_DIR = os.path.abspath(os.path.join(INFRA_BOTS_DIR, os.pardir, os.pardir))

CIPD_PACKAGE_NAME_TMPL = 'skia/bots/%s'
DEFAULT_CIPD_SERVICE_URL = 'https://chrome-infra-packages.appspot.com'

DEFAULT_GS_BUCKET = 'skia-buildbots'
GS_SUBDIR_TMPL = 'gs://%s/assets/%s'
GS_PATH_TMPL = '%s/%s.zip'

TAG_PROJECT_SKIA = 'project:skia'
TAG_VERSION_PREFIX = 'version:'
TAG_VERSION_TMPL = '%s%%s' % TAG_VERSION_PREFIX

VERSION_FILENAME = 'VERSION'
ZIP_BLACKLIST = ['.git', '.svn', '*.pyc', '.DS_STORE']


class CIPDStore(object):
  """Wrapper object for CIPD."""
  def __init__(self, cipd_url=DEFAULT_CIPD_SERVICE_URL):
    cipd = 'cipd'
    platform = 'linux64'
    if sys.platform == 'darwin':
      platform = 'mac64'
    elif sys.platform == 'win32':
      platform = 'win64'
      cipd = 'cipd.exe'
    self._cipd_path = os.path.join(INFRA_BOTS_DIR, 'tools', 'luci-go', platform)
    self._cipd = os.path.join(self._cipd_path, cipd)
    self._cipd_url = cipd_url
    self._check_setup()

  def _check_setup(self):
    """Verify that we have the CIPD binary and that we're authenticated."""
    try:
      subprocess.check_call([self._cipd, 'auth-info'])
    except OSError:
      cipd_sha1_path = os.path.join(self._cipd_path, 'cipd.sha1')
      raise Exception('CIPD binary not found in %s. You may need to run:\n\n'
                      '$ download_from_google_storage -s %s'
                      ' --bucket chromium-luci' % (self._cipd, cipd_sha1_path))
    except subprocess.CalledProcessError:
      raise Exception('CIPD not authenticated. You may need to run:\n\n'
                      '$ %s auth-login' % self._cipd)

  def _run(self, cmd):
    """Run the given command."""
    subprocess.check_call(
        [self._cipd]
        + cmd
        + ['--service-url', self._cipd_url]
    )

  def _json_output(self, cmd):
    """Run the given command, return the JSON output."""
    with utils.tmp_dir():
      json_output = os.path.join(os.getcwd(), 'output.json')
      self._run(cmd + ['--json-output', json_output])
      with open(json_output) as f:
        parsed = json.load(f)
    return parsed.get('result', [])

  def _search(self, pkg_name):
    res = self._json_output(['search', pkg_name, '--tag', TAG_PROJECT_SKIA])
    return [r['instance_id'] for r in res]

  def _describe(self, pkg_name, instance_id):
    """Obtain details about the given package and instance ID."""
    return self._json_output(['describe', pkg_name, '--version', instance_id])

  def get_available_versions(self, name):
    """List available versions of the asset."""
    pkg_name = CIPD_PACKAGE_NAME_TMPL % name
    versions = []
    for instance_id in self._search(pkg_name):
      details = self._describe(pkg_name, instance_id)
      for tag in details.get('tags'):
        tag_name = tag.get('tag', '')
        if tag_name.startswith(TAG_VERSION_PREFIX):
          trimmed = tag_name[len(TAG_VERSION_PREFIX):]
          try:
            versions.append(int(trimmed))
          except ValueError:
            raise ValueError('Found package instance with invalid version '
                             'tag: %s' % tag_name)
    versions.sort()
    return versions

  def upload(self, name, version, target_dir):
    """Create a CIPD package."""
    self._run([
        'create',
        '--name', CIPD_PACKAGE_NAME_TMPL % name,
        '--in', target_dir,
        '--tag', TAG_PROJECT_SKIA,
        '--tag', TAG_VERSION_TMPL % version,
    ])

  def download(self, name, version, target_dir):
    """Download a CIPD package."""
    pkg_name = CIPD_PACKAGE_NAME_TMPL % name
    version_tag = TAG_VERSION_TMPL % version
    target_dir = os.path.abspath(target_dir)
    with utils.tmp_dir():
      infile = os.path.join(os.getcwd(), 'input')
      with open(infile, 'w') as f:
        f.write('%s %s' % (pkg_name, version_tag))
      self._run([
          'ensure',
          '--root', target_dir,
          '--list', infile,
      ])

  def delete_contents(self, name):
    """Delete data for the given asset."""
    self._run(['pkg-delete', CIPD_PACKAGE_NAME_TMPL % name])


class GSStore(object):
  """Wrapper object for interacting with Google Storage."""
  def __init__(self, gsutil=None, bucket=DEFAULT_GS_BUCKET):
    gsutil = os.path.abspath(gsutil) if gsutil else 'gsutil'
    self._gsutil = [gsutil]
    if gsutil.endswith('.py'):
      self._gsutil = ['python', gsutil]
    self._gs_bucket = bucket

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

  def get_available_versions(self, name):
    """Return the existing version numbers for the asset."""
    files = self.list(GS_SUBDIR_TMPL % (self._gs_bucket, name))
    bnames = [os.path.basename(f) for f in files]
    suffix = '.zip'
    versions = [int(f[:-len(suffix)]) for f in bnames if f.endswith(suffix)]
    versions.sort()
    return versions

  def upload(self, name, version, target_dir):
    """Upload to GS."""
    target_dir = os.path.abspath(target_dir)
    with utils.tmp_dir():
      zip_file = os.path.join(os.getcwd(), '%d.zip' % version)
      zip_utils.zip(target_dir, zip_file, blacklist=ZIP_BLACKLIST)
      gs_path = GS_PATH_TMPL % (GS_SUBDIR_TMPL % (self._gs_bucket, name),
                                str(version))
      self.copy(zip_file, gs_path)

  def download(self, name, version, target_dir):
    """Download from GS."""
    gs_path = GS_PATH_TMPL % (GS_SUBDIR_TMPL % (self._gs_bucket, name),
                              str(version))
    target_dir = os.path.abspath(target_dir)
    with utils.tmp_dir():
      zip_file = os.path.join(os.getcwd(), '%d.zip' % version)
      self.copy(gs_path, zip_file)
      zip_utils.unzip(zip_file, target_dir)

  def delete_contents(self, name):
    """Delete data for the given asset."""
    gs_path = GS_SUBDIR_TMPL % (self._gs_bucket, name)
    attempt_delete = True
    try:
      subprocess.check_call(['gsutil', 'ls', gs_path])
    except subprocess.CalledProcessError:
      attempt_delete = False
    if attempt_delete:
      subprocess.check_call(['gsutil', 'rm', '-rf', gs_path])


class MultiStore(object):
  """Wrapper object which uses CIPD as the primary store and GS for backup."""
  def __init__(self, cipd_url=DEFAULT_CIPD_SERVICE_URL,
               gsutil=None, gs_bucket=DEFAULT_GS_BUCKET):
    self._cipd = CIPDStore(cipd_url=cipd_url)
    self._gs = GSStore(gsutil=gsutil, bucket=gs_bucket)

  def get_available_versions(self, name):
    return self._cipd.get_available_versions(name)

  def upload(self, name, version, target_dir):
    self._cipd.upload(name, version, target_dir)
    self._gs.upload(name, version, target_dir)

  def download(self, name, version, target_dir):
    self._cipd.download(name, version, target_dir)

  def delete_contents(self, name):
    self._cipd.delete_contents(name)
    self._gs.delete_contents(name)


def _prompt(prompt):
  """Prompt for input, return result."""
  return raw_input(prompt)


class Asset(object):
  def __init__(self, name, store):
    self._store = store
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
    return self._store.get_available_versions(self._name)

  def get_next_version(self):
    """Find the next available version number for the asset."""
    versions = self.get_available_versions()
    if len(versions) == 0:
      return 0
    return versions[-1] + 1

  def download_version(self, version, target_dir):
    """Download the specified version of the asset."""
    self._store.download(self._name, version, target_dir)

  def download_current_version(self, target_dir):
    """Download the version of the asset specified in its version file."""
    v = self.get_current_version()
    self.download_version(v, target_dir)

  def upload_new_version(self, target_dir, commit=False):
    """Upload a new version and update the version file for the asset."""
    version = self.get_next_version()
    self._store.upload(self._name, version, target_dir)

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
  def add(cls, name, store):
    """Add an asset."""
    asset = cls(name, store)
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

  def remove(self, remove_in_store=False):
    """Remove this asset."""
    # Ensure that the asset exists.
    if not os.path.isdir(self._dir):
      raise Exception('Asset %s does not exist!' % self._name)

    # Cleanup the store.
    if remove_in_store:
      self._store.delete_contents(self._name)

    # Remove the asset.
    subprocess.check_call([utils.GIT, 'rm', '-rf', self._dir])
    if os.path.isdir(self._dir):
      shutil.rmtree(self._dir)
