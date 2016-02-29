#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import contextlib
import math
import os
import shutil
import socket
import subprocess
import sys
import time
import urllib2

from flavor import android_flavor
from flavor import chromeos_flavor
from flavor import cmake_flavor
from flavor import coverage_flavor
from flavor import default_flavor
from flavor import ios_flavor
from flavor import valgrind_flavor
from flavor import xsan_flavor


CONFIG_COVERAGE = 'Coverage'
CONFIG_DEBUG = 'Debug'
CONFIG_RELEASE = 'Release'
VALID_CONFIGS = (CONFIG_COVERAGE, CONFIG_DEBUG, CONFIG_RELEASE)

GM_ACTUAL_FILENAME = 'actual-results.json'
GM_EXPECTATIONS_FILENAME = 'expected-results.json'
GM_IGNORE_TESTS_FILENAME = 'ignored-tests.txt'

GOLD_UNINTERESTING_HASHES_URL = 'https://gold.skia.org/_/hashes'

GS_GM_BUCKET = 'chromium-skia-gm'
GS_SUMMARIES_BUCKET = 'chromium-skia-gm-summaries'

GS_SUBDIR_TMPL_SK_IMAGE = 'skimage/v%s'
GS_SUBDIR_TMPL_SKP = 'playback_%s/skps'

SKIA_REPO = 'https://skia.googlesource.com/skia.git'
INFRA_REPO = 'https://skia.googlesource.com/buildbot.git'

SERVICE_ACCOUNT_FILE = 'service-account-skia.json'
SERVICE_ACCOUNT_INTERNAL_FILE = 'service-account-skia-internal.json'

VERSION_FILE_SK_IMAGE = 'SK_IMAGE_VERSION'
VERSION_FILE_SKP = 'SKP_VERSION'


def is_android(bot_cfg):
  """Determine whether the given bot is an Android bot."""
  return ('Android' in bot_cfg.get('extra_config', '') or
          bot_cfg.get('os') == 'Android')

def is_chromeos(bot_cfg):
  return ('CrOS' in bot_cfg.get('extra_config', '') or
          bot_cfg.get('os') == 'ChromeOS')

def is_cmake(bot_cfg):
  return 'CMake' in bot_cfg.get('extra_config', '')

def is_ios(bot_cfg):
  return ('iOS' in bot_cfg.get('extra_config', '') or
          bot_cfg.get('os') == 'iOS')


def is_valgrind(bot_cfg):
  return 'Valgrind' in bot_cfg.get('extra_config', '')


def is_xsan(bot_cfg):
  return (bot_cfg.get('extra_config') == 'ASAN' or
          bot_cfg.get('extra_config') == 'MSAN' or
          bot_cfg.get('extra_config') == 'TSAN')


def download_dir(skia_dir, tmp_dir, version_file, gs_path_tmpl, dst_dir):
  # Ensure that the tmp_dir exists.
  if not os.path.isdir(tmp_dir):
    os.makedirs(tmp_dir)

  # Get the expected version.
  with open(os.path.join(skia_dir, version_file)) as f:
    expected_version = f.read().rstrip()

  print 'Expected %s = %s' % (version_file, expected_version)

  # Get the actually-downloaded version, if we have one.
  actual_version_file = os.path.join(tmp_dir, version_file)
  try:
    with open(actual_version_file) as f:
      actual_version = f.read().rstrip()
  except IOError:
    actual_version = -1

  print 'Actual   %s = %s' % (version_file, actual_version)

  # If we don't have the desired version, download it.
  if actual_version != expected_version:
    if actual_version != -1:
      os.remove(actual_version_file)
    if os.path.isdir(dst_dir):
      shutil.rmtree(dst_dir)
    os.makedirs(dst_dir)
    gs_path = 'gs://%s/%s/*' % (GS_GM_BUCKET, gs_path_tmpl % expected_version)
    print 'Downloading from %s' % gs_path
    subprocess.check_call(['gsutil', 'cp', '-R', gs_path, dst_dir])
    with open(actual_version_file, 'w') as f:
      f.write(expected_version)


def get_uninteresting_hashes(hashes_file):
  retries = 5
  timeout = 60
  wait_base = 15

  socket.setdefaulttimeout(timeout)
  for retry in range(retries):
    try:
      with contextlib.closing(
          urllib2.urlopen(GOLD_UNINTERESTING_HASHES_URL, timeout=timeout)) as w:
        hashes = w.read()
        with open(hashes_file, 'w') as f:
          f.write(hashes)
          break
    except Exception as e:
      print >> sys.stderr, 'Failed to get uninteresting hashes from %s:\n%s' % (
          GOLD_UNINTERESTING_HASHES_URL, e)
      if retry == retries:
        raise
      waittime = wait_base * math.pow(2, retry)
      print 'Retry in %d seconds.' % waittime
      time.sleep(waittime)


class BotInfo(object):
  def __init__(self, bot_name, swarm_out_dir):
    """Initialize the bot, given its name.

    Assumes that CWD is the directory containing this file.
    """
    self.name = bot_name
    self.skia_dir = os.path.abspath(os.path.join(
        os.path.dirname(os.path.realpath(__file__)),
        os.pardir, os.pardir))
    self.swarm_out_dir = swarm_out_dir
    os.chdir(self.skia_dir)
    self.build_dir = os.path.abspath(os.path.join(self.skia_dir, os.pardir))
    self.spec = self.get_bot_spec(bot_name)
    self.bot_cfg = self.spec['builder_cfg']
    if self.bot_cfg['role'] == 'Build':
      self.out_dir = os.path.join(swarm_out_dir, 'out')
    else:
      self.out_dir = 'out'
    self.configuration = self.spec['configuration']
    self.default_env = {
      'SKIA_OUT': self.out_dir,
      'BUILDTYPE': self.configuration,
      'PATH': os.environ['PATH'],
    }
    self.default_env.update(self.spec['env'])
    self.build_targets = [str(t) for t in self.spec['build_targets']]
    self.is_trybot = self.bot_cfg['is_trybot']
    self.upload_dm_results = self.spec['upload_dm_results']
    self.upload_perf_results = self.spec['upload_perf_results']
    self.perf_data_dir = os.path.join(self.swarm_out_dir, 'perfdata',
                                      self.name, 'data')
    self.resource_dir = os.path.join(self.build_dir, 'resources')
    self.images_dir = os.path.join(self.build_dir, 'images')
    self.local_skp_dir = os.path.join(self.build_dir, 'playback', 'skps')
    self.dm_flags = self.spec['dm_flags']
    self.nanobench_flags = self.spec['nanobench_flags']
    self._ccache = None
    self._checked_for_ccache = False
    self._already_ran = {}
    self.tmp_dir = os.path.join(self.build_dir, 'tmp')
    self.flavor = self.get_flavor(self.bot_cfg)

    # These get filled in during subsequent steps.
    self.device_dirs = None
    self.build_number = None
    self.got_revision = None
    self.master_name = None
    self.slave_name = None

  @property
  def ccache(self):
    if not self._checked_for_ccache:
      self._checked_for_ccache = True
      if sys.platform != 'win32':
        try:
          result = subprocess.check_output(['which', 'ccache'])
          self._ccache = result.rstrip()
        except subprocess.CalledProcessError:
          pass

    return self._ccache

  def get_bot_spec(self, bot_name):
    """Retrieve the bot spec for this bot."""
    sys.path.append(self.skia_dir)
    from tools import buildbot_spec
    return buildbot_spec.get_builder_spec(bot_name)

  def get_flavor(self, bot_cfg):
    """Return a flavor utils object specific to the given bot."""
    if is_android(bot_cfg):
      return android_flavor.AndroidFlavorUtils(self)
    elif is_chromeos(bot_cfg):
      return chromeos_flavor.ChromeOSFlavorUtils(self)
    elif is_cmake(bot_cfg):
      return cmake_flavor.CMakeFlavorUtils(self)
    elif is_ios(bot_cfg):
      return ios_flavor.iOSFlavorUtils(self)
    elif is_valgrind(bot_cfg):
      return valgrind_flavor.ValgrindFlavorUtils(self)
    elif is_xsan(bot_cfg):
      return xsan_flavor.XSanFlavorUtils(self)
    elif bot_cfg.get('configuration') == CONFIG_COVERAGE:
      return coverage_flavor.CoverageFlavorUtils(self)
    else:
      return default_flavor.DefaultFlavorUtils(self)

  def run(self, cmd, env=None, cwd=None):
    _env = {}
    _env.update(self.default_env)
    _env.update(env or {})
    cwd = cwd or self.skia_dir
    print '============'
    print 'CMD: %s' % cmd
    print 'CWD: %s' % cwd
    print 'ENV: %s' % _env
    print '============'
    subprocess.check_call(cmd, env=_env, cwd=cwd)

  def compile_steps(self):
    for t in self.build_targets:
      self.flavor.compile(t)

  def _run_once(self, fn, *args, **kwargs):
    if not fn.__name__ in self._already_ran:
      self._already_ran[fn.__name__] = True
      fn(*args, **kwargs)

  def install(self):
    """Copy the required executables and files to the device."""
    self.device_dirs = self.flavor.get_device_dirs()

    # Run any device-specific installation.
    self.flavor.install()

    # TODO(borenet): Only copy files which have changed.
    # Resources
    self.flavor.copy_directory_contents_to_device(self.resource_dir,
                                                  self.device_dirs.resource_dir)

  def _key_params(self):
    """Build a unique key from the builder name (as a list).

    E.g.  arch x86 gpu GeForce320M mode MacMini4.1 os Mac10.6
    """
    # Don't bother to include role, which is always Test.
    # TryBots are uploaded elsewhere so they can use the same key.
    blacklist = ['role', 'is_trybot']

    flat = []
    for k in sorted(self.bot_cfg.keys()):
      if k not in blacklist:
        flat.append(k)
        flat.append(self.bot_cfg[k])
    return flat

  def test_steps(self, got_revision, master_name, slave_name, build_number):
    """Run the DM test."""
    self.build_number = build_number
    self.got_revision = got_revision
    self.master_name = master_name
    self.slave_name = slave_name
    self._run_once(self.install)

    use_hash_file = False
    if self.upload_dm_results:
      # This must run before we write anything into self.device_dirs.dm_dir
      # or we may end up deleting our output on machines where they're the same.
      host_dm_dir = os.path.join(self.swarm_out_dir, 'dm')
      print 'host dm dir: %s' % host_dm_dir
      self.flavor.create_clean_host_dir(host_dm_dir)
      if str(host_dm_dir) != str(self.device_dirs.dm_dir):
        self.flavor.create_clean_device_dir(self.device_dirs.dm_dir)

      # Obtain the list of already-generated hashes.
      hash_filename = 'uninteresting_hashes.txt'
      host_hashes_file = os.path.join(self.tmp_dir, hash_filename)
      hashes_file = self.flavor.device_path_join(
          self.device_dirs.tmp_dir, hash_filename)

      try:
        get_uninteresting_hashes(host_hashes_file)
      except Exception:
        pass

      if os.path.exists(host_hashes_file):
        self.flavor.copy_file_to_device(host_hashes_file, hashes_file)
        use_hash_file = True

    # Run DM.
    properties = [
      'gitHash',      self.got_revision,
      'master',       self.master_name,
      'builder',      self.name,
      'build_number', self.build_number,
    ]
    if self.is_trybot:
      properties.extend([
        'issue',    self.m.properties['issue'],
        'patchset', self.m.properties['patchset'],
      ])

    args = [
      'dm',
      '--undefok',   # This helps branches that may not know new flags.
      '--verbose',
      '--resourcePath', self.device_dirs.resource_dir,
      '--skps', self.device_dirs.skp_dir,
      '--images', self.flavor.device_path_join(
          self.device_dirs.images_dir, 'dm'),
      '--nameByHash',
      '--properties'
    ] + properties

    args.append('--key')
    args.extend(self._key_params())
    if use_hash_file:
      args.extend(['--uninterestingHashesFile', hashes_file])
    if self.upload_dm_results:
      args.extend(['--writePath', self.device_dirs.dm_dir])

    skip_flag = None
    if self.bot_cfg.get('cpu_or_gpu') == 'CPU':
      skip_flag = '--nogpu'
    elif self.bot_cfg.get('cpu_or_gpu') == 'GPU':
      skip_flag = '--nocpu'
    if skip_flag:
      args.append(skip_flag)
    args.extend(self.dm_flags)

    self.flavor.run(args, env=self.default_env)

    if self.upload_dm_results:
      # Copy images and JSON to host machine if needed.
      self.flavor.copy_directory_contents_to_host(self.device_dirs.dm_dir,
                                                  host_dm_dir)

    # See skia:2789.
    if ('Valgrind' in self.name and
        self.builder_cfg.get('cpu_or_gpu') == 'GPU'):
      abandonGpuContext = list(args)
      abandonGpuContext.append('--abandonGpuContext')
      self.flavor.run(abandonGpuContext)
      preAbandonGpuContext = list(args)
      preAbandonGpuContext.append('--preAbandonGpuContext')
      self.flavor.run(preAbandonGpuContext)
