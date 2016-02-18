#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import os
import subprocess
import sys

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

GS_GM_BUCKET = 'chromium-skia-gm'
GS_SUMMARIES_BUCKET = 'chromium-skia-gm-summaries'

SKIA_REPO = 'https://skia.googlesource.com/skia.git'
INFRA_REPO = 'https://skia.googlesource.com/buildbot.git'

SERVICE_ACCOUNT_FILE = 'service-account-skia.json'
SERVICE_ACCOUNT_INTERNAL_FILE = 'service-account-skia-internal.json'


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


class BotInfo(object):
  def __init__(self, bot_name, slave_name, out_dir):
    """Initialize the bot, given its name.

    Assumes that CWD is the directory containing this file.
    """
    self.name = bot_name
    self.slave_name = slave_name
    self.skia_dir = os.path.abspath(os.path.join(
        os.path.dirname(os.path.realpath(__file__)),
        os.pardir, os.pardir))
    os.chdir(self.skia_dir)
    self.build_dir = os.path.abspath(os.path.join(self.skia_dir, os.pardir))
    self.out_dir = out_dir
    self.spec = self.get_bot_spec(bot_name)
    self.configuration = self.spec['configuration']
    self.default_env = {
      'SKIA_OUT': self.out_dir,
      'BUILDTYPE': self.configuration,
      'PATH': os.environ['PATH'],
    }
    self.default_env.update(self.spec['env'])
    self.build_targets = [str(t) for t in self.spec['build_targets']]
    self.bot_cfg = self.spec['builder_cfg']
    self.is_trybot = self.bot_cfg['is_trybot']
    self.upload_dm_results = self.spec['upload_dm_results']
    self.upload_perf_results = self.spec['upload_perf_results']
    self.dm_flags = self.spec['dm_flags']
    self.nanobench_flags = self.spec['nanobench_flags']
    self._ccache = None
    self._checked_for_ccache = False
    self.flavor = self.get_flavor(self.bot_cfg)

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
