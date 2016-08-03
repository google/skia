# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


import json
import os
import re
import sys

from recipe_engine import recipe_api
from recipe_engine import config_types

from . import fake_specs


TEST_EXPECTED_SKP_VERSION = '42'
TEST_EXPECTED_SK_IMAGE_VERSION = '42'

VERSION_FILE_SK_IMAGE = 'SK_IMAGE_VERSION'
VERSION_FILE_SKP = 'SKP_VERSION'

VERSION_NONE = -1


class SkiaApi(recipe_api.RecipeApi):

  def get_builder_spec(self, skia_dir, builder_name):
    """Obtain the buildbot spec for the given builder."""
    fake_spec = None
    if self._test_data.enabled:
      fake_spec = fake_specs.FAKE_SPECS[builder_name]
    builder_spec = self.m.run.json_from_file(
      skia_dir.join('tools', 'buildbot_spec.py'),
      skia_dir,
      builder_name,
      fake_spec)
    return builder_spec

  def setup(self):
    """Prepare the bot to run."""
    # Setup dependencies.
    self.m.vars.setup()

    # Check out the Skia code.
    self.checkout_steps()

    # Obtain the spec for this builder from the Skia repo. Use it to set more
    # properties.
    builder_spec = self.get_builder_spec(self.m.vars.skia_dir,
                                         self.m.vars.builder_name)

    # Continue setting up vars with the builder_spec.
    self.m.vars.update_with_builder_spec(builder_spec)

    
    if not self.m.path.exists(self.m.vars.tmp_dir):
      self.m.run.run_once(self.m.file.makedirs,
                                'tmp_dir',
                                self.m.vars.tmp_dir,
                                infra_step=True)

    self.m.flavor.setup()

  def update_repo(self, parent_dir, repo):
    """Update an existing repo. This is safe to call without gen_steps."""
    repo_path = parent_dir.join(repo.name)
    if self.m.path.exists(repo_path):  # pragma: nocover
      if self.m.platform.is_win:
        git = 'git.bat'
      else:
        git = 'git'
      self.m.step('git remote set-url',
                  cmd=[git, 'remote', 'set-url', 'origin', repo.url],
                  cwd=repo_path,
                  infra_step=True)
      self.m.step('git fetch',
                  cmd=[git, 'fetch'],
                  cwd=repo_path,
                  infra_step=True)
      self.m.step('git reset',
                  cmd=[git, 'reset', '--hard', repo.revision],
                  cwd=repo_path,
                  infra_step=True)
      self.m.step('git clean',
                  cmd=[git, 'clean', '-d', '-f'],
                  cwd=repo_path,
                  infra_step=True)

  def checkout_steps(self):
    """Run the steps to obtain a checkout of Skia."""
    cfg_kwargs = {}
    if not self.m.vars.persistent_checkout:
      # We should've obtained the Skia checkout through isolates, so we don't
      # need to perform the checkout ourselves.
      return

    # Use a persistent gclient cache for Swarming.
    cfg_kwargs['CACHE_DIR'] = self.m.vars.gclient_cache

    # Create the checkout path if necessary.
    if not self.m.path.exists(self.m.vars.checkout_root):
      self.m.file.makedirs('checkout_path',
                           self.m.vars.checkout_root,
                           infra_step=True)

    # Initial cleanup.
    gclient_cfg = self.m.gclient.make_config(**cfg_kwargs)
    skia = gclient_cfg.solutions.add()
    skia.name = 'skia'
    skia.managed = False
    skia.url = 'https://skia.googlesource.com/skia.git'
    skia.revision = self.m.properties.get('revision') or 'origin/master'
    self.update_repo(self.m.vars.checkout_root, skia)

    # TODO(rmistry): Remove the below block after there is a solution for
    #                crbug.com/616443
    entries_file = self.m.vars.checkout_root.join('.gclient_entries')
    if self.m.path.exists(entries_file):
      self.m.file.remove('remove %s' % entries_file,
                         entries_file,
                         infra_step=True)  # pragma: no cover

    if self.m.vars.need_chromium_checkout:
      chromium = gclient_cfg.solutions.add()
      chromium.name = 'src'
      chromium.managed = False
      chromium.url = 'https://chromium.googlesource.com/chromium/src.git'
      chromium.revision = 'origin/lkgr'
      self.update_repo(self.m.vars.checkout_root, chromium)

    if self.m.vars.need_pdfium_checkout:
      pdfium = gclient_cfg.solutions.add()
      pdfium.name = 'pdfium'
      pdfium.managed = False
      pdfium.url = 'https://pdfium.googlesource.com/pdfium.git'
      pdfium.revision = 'origin/master'
      self.update_repo(self.m.vars.checkout_root, pdfium)

    # Run 'gclient sync'.
    gclient_cfg.got_revision_mapping['skia'] = 'got_revision'
    gclient_cfg.target_os.add('llvm')
    checkout_kwargs = {}
    checkout_kwargs['env'] = self.m.vars.default_env

    # api.gclient.revert() assumes things about the layout of the code, so it
    # fails for us. Run an appropriate revert sequence for trybots instead.
    gclient_file = self.m.vars.checkout_root.join('.gclient')
    if (self.m.tryserver.is_tryserver and
        self.m.path.exists(gclient_file)):  # pragma: no cover
      # These steps taken from:
      # https://chromium.googlesource.com/chromium/tools/build/+/
      #    81a696760ab7c25f6606c54fc781b90b8af9fdd2/scripts/slave/
      #    gclient_safe_revert.py
      if self.m.path.exists(entries_file):
        self.m.gclient('recurse', [
            'recurse', '-i', 'sh', '-c',
            'if [ -e .git ]; then git remote update; fi'])
      self.m.gclient(
          'revert',
          ['revert', '-v', '-v', '-v', '--nohooks', '--upstream'],
          cwd=self.m.vars.checkout_root)

    update_step = self.m.gclient.checkout(gclient_config=gclient_cfg,
                                          cwd=self.m.vars.checkout_root,
                                          revert=False,
                                          **checkout_kwargs)

    self.m.vars.got_revision = (
        update_step.presentation.properties['got_revision'])
    self.m.tryserver.maybe_apply_issue()

    if self.m.vars.need_chromium_checkout:
      self.m.gclient.runhooks(cwd=self.m.vars.checkout_root,
                              env=self.m.vars.gclient_env)

  def copy_dir(self, host_version, version_file, tmp_dir,
               host_path, device_path, test_expected_version,
               test_actual_version):
    actual_version_file = self.m.path.join(tmp_dir, version_file)
    # Copy to device.
    device_version_file = self.m.flavor.device_path_join(
        self.m.flavor.device_dirs.tmp_dir, version_file)
    if str(actual_version_file) != str(device_version_file):
      try:
        device_version = (
            self.m.flavor.read_file_on_device(device_version_file))
      except self.m.step.StepFailure:
        device_version = VERSION_NONE
      if device_version != host_version:
        self.m.flavor.remove_file_on_device(device_version_file)
        self.m.flavor.create_clean_device_dir(device_path)
        self.m.flavor.copy_directory_contents_to_device(
            host_path, device_path)

        # Copy the new version file.
        self.m.flavor.copy_file_to_device(actual_version_file,
                                        device_version_file)

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
    self.copy_dir(
        version,
        VERSION_FILE_SK_IMAGE,
        self.m.vars.tmp_dir,
        self.m.vars.images_dir,
        self.m.flavor.device_dirs.images_dir,
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
    self.copy_dir(
        version,
        VERSION_FILE_SKP,
        self.m.vars.tmp_dir,
        self.m.vars.local_skp_dir,
        self.m.flavor.device_dirs.skp_dir,
        test_expected_version=self.m.properties.get(
            'test_downloaded_skp_version', TEST_EXPECTED_SKP_VERSION),
        test_actual_version=self.m.properties.get(
            'test_downloaded_skp_version', TEST_EXPECTED_SKP_VERSION))
    return version

  def install(self):
    """Copy the required executables and files to the device."""
    # Run any device-specific installation.
    self.m.flavor.install()

    # TODO(borenet): Only copy files which have changed.
    # Resources
    self.m.flavor.copy_directory_contents_to_device(
        self.m.vars.resource_dir,
        self.m.flavor.device_dirs.resource_dir)

  def test_steps(self):
    """Run the DM test."""
    self.m.run.run_once(self.install)
    self.m.run.run_once(self._copy_skps)
    self.m.run.run_once(self._copy_images)

    use_hash_file = False
    if self.m.vars.upload_dm_results:
      # This must run before we write anything into
      # self.m.flavor.device_dirs.dm_dir or we may end up deleting our
      # output on machines where they're the same.
      self.m.flavor.create_clean_host_dir(self.m.vars.dm_dir)
      host_dm_dir = str(self.m.vars.dm_dir)
      device_dm_dir = str(self.m.flavor.device_dirs.dm_dir)
      if host_dm_dir != device_dm_dir:
        self.m.flavor.create_clean_device_dir(device_dm_dir)

      # Obtain the list of already-generated hashes.
      hash_filename = 'uninteresting_hashes.txt'

      # Ensure that the tmp_dir exists.
      self.m.run.run_once(self.m.file.makedirs,
                     'tmp_dir',
                     self.m.vars.tmp_dir,
                     infra_step=True)

      host_hashes_file = self.m.vars.tmp_dir.join(hash_filename)
      hashes_file = self.m.flavor.device_path_join(
          self.m.flavor.device_dirs.tmp_dir, hash_filename)
      self.m.run(
          self.m.python.inline,
          'get uninteresting hashes',
          program="""
          import contextlib
          import math
          import socket
          import sys
          import time
          import urllib2

          HASHES_URL = 'https://gold.skia.org/_/hashes'
          RETRIES = 5
          TIMEOUT = 60
          WAIT_BASE = 15

          socket.setdefaulttimeout(TIMEOUT)
          for retry in range(RETRIES):
            try:
              with contextlib.closing(
                  urllib2.urlopen(HASHES_URL, timeout=TIMEOUT)) as w:
                hashes = w.read()
                with open(sys.argv[1], 'w') as f:
                  f.write(hashes)
                  break
            except Exception as e:
              print 'Failed to get uninteresting hashes from %s:' % HASHES_URL
              print e
              if retry == RETRIES:
                raise
              waittime = WAIT_BASE * math.pow(2, retry)
              print 'Retry in %d seconds.' % waittime
              time.sleep(waittime)
          """,
          args=[host_hashes_file],
          cwd=self.m.vars.skia_dir,
          abort_on_failure=False,
          fail_build_on_failure=False,
          infra_step=True)

      if self.m.path.exists(host_hashes_file):
        self.m.flavor.copy_file_to_device(host_hashes_file, hashes_file)
        use_hash_file = True

    # Run DM.
    properties = [
      'gitHash',      self.m.vars.got_revision,
      'master',       self.m.vars.master_name,
      'builder',      self.m.vars.builder_name,
      'build_number', self.m.vars.build_number,
    ]
    if self.m.vars.is_trybot:
      properties.extend([
        'issue',    self.m.vars.issue,
        'patchset', self.m.vars.patchset,
      ])

    args = [
      'dm',
      '--undefok',   # This helps branches that may not know new flags.
      '--resourcePath', self.m.flavor.device_dirs.resource_dir,
      '--skps', self.m.flavor.device_dirs.skp_dir,
      '--images', self.m.flavor.device_path_join(
          self.m.flavor.device_dirs.images_dir, 'dm'),
      '--colorImages', self.m.flavor.device_path_join(
          self.m.flavor.device_dirs.images_dir, 'colorspace'),
      '--nameByHash',
      '--properties'
    ] + properties

    args.append('--key')
    args.extend(self._KeyParams())
    if use_hash_file:
      args.extend(['--uninterestingHashesFile', hashes_file])
    if self.m.vars.upload_dm_results:
      args.extend(['--writePath', self.m.flavor.device_dirs.dm_dir])

    skip_flag = None
    if self.m.vars.builder_cfg.get('cpu_or_gpu') == 'CPU':
      skip_flag = '--nogpu'
    elif self.m.vars.builder_cfg.get('cpu_or_gpu') == 'GPU':
      skip_flag = '--nocpu'
    if skip_flag:
      args.append(skip_flag)
    args.extend(self.m.vars.dm_flags)

    self.m.run(self.m.flavor.step, 'dm', cmd=args,
                     abort_on_failure=False,
                     env=self.m.vars.default_env)

    if self.m.vars.upload_dm_results:
      # Copy images and JSON to host machine if needed.
      self.m.flavor.copy_directory_contents_to_host(
          self.m.flavor.device_dirs.dm_dir, self.m.vars.dm_dir)

    # See skia:2789.
    if ('Valgrind' in self.m.vars.builder_name and
        self.m.vars.builder_cfg.get('cpu_or_gpu') == 'GPU'):
      abandonGpuContext = list(args)
      abandonGpuContext.append('--abandonGpuContext')
      self.m.run(self.m.flavor.step, 'dm --abandonGpuContext',
                       cmd=abandonGpuContext, abort_on_failure=False)
      preAbandonGpuContext = list(args)
      preAbandonGpuContext.append('--preAbandonGpuContext')
      self.m.run(self.m.flavor.step, 'dm --preAbandonGpuContext',
                       cmd=preAbandonGpuContext, abort_on_failure=False,
                       env=self.m.vars.default_env)

  def perf_steps(self):
    """Run Skia benchmarks."""
    self.m.run.run_once(self.install)
    self.m.run.run_once(self._copy_skps)
    self.m.run.run_once(self._copy_images)

    if self.m.vars.upload_perf_results:
      self.m.flavor.create_clean_device_dir(
          self.m.flavor.device_dirs.perf_data_dir)

    # Run nanobench.
    properties = [
      '--properties',
      'gitHash',      self.m.vars.got_revision,
      'build_number', self.m.vars.build_number,
    ]
    if self.m.vars.is_trybot:
      properties.extend([
        'issue',    self.m.vars.issue,
        'patchset', self.m.vars.patchset,
      ])

    target = 'nanobench'
    if 'VisualBench' in self.m.vars.builder_name:
      target = 'visualbench'
    args = [
        target,
        '--undefok',   # This helps branches that may not know new flags.
        '-i',       self.m.flavor.device_dirs.resource_dir,
        '--skps',   self.m.flavor.device_dirs.skp_dir,
        '--images', self.m.flavor.device_path_join(
            self.m.flavor.device_dirs.images_dir, 'nanobench'),
    ]

    skip_flag = None
    if self.m.vars.builder_cfg.get('cpu_or_gpu') == 'CPU':
      skip_flag = '--nogpu'
    elif self.m.vars.builder_cfg.get('cpu_or_gpu') == 'GPU':
      skip_flag = '--nocpu'
    if skip_flag:
      args.append(skip_flag)
    args.extend(self.m.vars.nanobench_flags)

    if self.m.vars.upload_perf_results:
      json_path = self.m.flavor.device_path_join(
          self.m.flavor.device_dirs.perf_data_dir,
          'nanobench_%s.json' % self.m.vars.got_revision)
      args.extend(['--outResultsFile', json_path])
      args.extend(properties)

      keys_blacklist = ['configuration', 'role', 'is_trybot']
      args.append('--key')
      for k in sorted(self.m.vars.builder_cfg.keys()):
        if not k in keys_blacklist:
          args.extend([k, self.m.vars.builder_cfg[k]])

    self.m.run(self.m.flavor.step, target, cmd=args,
                     abort_on_failure=False,
                     env=self.m.vars.default_env)

    # See skia:2789.
    if ('Valgrind' in self.m.vars.builder_name and
        self.m.vars.builder_cfg.get('cpu_or_gpu') == 'GPU'):
      abandonGpuContext = list(args)
      abandonGpuContext.extend(['--abandonGpuContext', '--nocpu'])
      self.m.run(self.m.flavor.step,
                       '%s --abandonGpuContext' % target,
                       cmd=abandonGpuContext, abort_on_failure=False,
                       env=self.m.vars.default_env)

    # Upload results.
    if self.m.vars.upload_perf_results:
      self.m.file.makedirs('perf_dir', self.m.vars.perf_data_dir)
      self.m.flavor.copy_directory_contents_to_host(
          self.m.flavor.device_dirs.perf_data_dir,
          self.m.vars.perf_data_dir)

  def cleanup_steps(self):
    """Run any cleanup steps."""
    self.m.flavor.cleanup_steps()

  def _KeyParams(self):
    """Build a unique key from the builder name (as a list).

    E.g.  arch x86 gpu GeForce320M mode MacMini4.1 os Mac10.6
    """
    # Don't bother to include role, which is always Test.
    # TryBots are uploaded elsewhere so they can use the same key.
    blacklist = ['role', 'is_trybot']

    flat = []
    for k in sorted(self.m.vars.builder_cfg.keys()):
      if k not in blacklist:
        flat.append(k)
        flat.append(self.m.vars.builder_cfg[k])
    return flat
