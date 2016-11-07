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


class SkiaApi(recipe_api.RecipeApi):

  def setup(self):
    """Prepare the bot to run."""
    # Setup dependencies.
    self.m.vars.setup()

    # Check out the Skia code.
    self.checkout_steps()

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

    # Run bot_update.
    gclient_cfg.got_revision_mapping['skia'] = 'got_revision'
    checkout_kwargs = {}
    checkout_kwargs['env'] = self.m.vars.default_env

    # Hack the patch ref if necessary.
    if self.m.properties.get('patch_storage', '') == 'gerrit':
      if self.m.bot_update._issue and self.m.bot_update._patchset:
        self.m.bot_update._gerrit_ref = 'refs/changes/%s/%d/%d' % (
            str(self.m.bot_update._issue)[-2:],
            self.m.bot_update._issue,
            self.m.bot_update._patchset,
        )

    update_step = self.m.bot_update.ensure_checkout(
        gclient_config=gclient_cfg,
        cwd=self.m.vars.checkout_root,
        **checkout_kwargs)

    self.m.vars.got_revision = (
        update_step.presentation.properties['got_revision'])

    if self.m.vars.need_chromium_checkout:
      self.m.gclient.runhooks(cwd=self.m.vars.checkout_root,
                              env=self.m.vars.gclient_env)
