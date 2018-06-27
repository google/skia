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

  def setup(self, bot_update=True):
    """Prepare the bot to run."""
    # Setup dependencies.
    self.m.vars.setup()

    # Check out the Skia code.
    if bot_update:
      self.checkout_bot_update()
    else:
      self.checkout_git()

    if not self.m.path.exists(self.m.vars.tmp_dir):
      self.m.run.run_once(self.m.file.ensure_directory,
                          'makedirs tmp_dir',
                          self.m.vars.tmp_dir)

    self.m.flavor.setup()

  def patch_ref(self, issue, patchset):
    """Build a ref for the given issue and patchset."""
    return 'refs/changes/%s/%s/%s' % (issue[-2:], issue, patchset)

  def checkout_git(self):
    """Run the steps to perform a pure-git checkout without DEPS."""
    self.m.git.checkout(
        self.m.properties['repository'], dir_path=self.m.vars.skia_dir,
        ref=self.m.properties['revision'], submodules=False)
    if self.m.vars.is_trybot:
      ref = self.patch_ref(str(self.m.vars.issue), str(self.m.vars.patchset))
      self.m.git('fetch', 'origin', ref)
      self.m.git('checkout', 'FETCH_HEAD')
      self.m.git('rebase', self.m.properties['revision'])

  def checkout_bot_update(self):
    """Run the steps to obtain a checkout using bot_update."""
    cfg_kwargs = {}
    is_parent_revision = 'ParentRevision' in self.m.vars.extra_tokens
    if not self.m.vars.persistent_checkout:
      assert not is_parent_revision
      # We should've obtained the Skia checkout through isolates, so we don't
      # need to perform the checkout ourselves.
      return

    # Use a persistent gclient cache for Swarming.
    cfg_kwargs['CACHE_DIR'] = self.m.vars.gclient_cache

    # Create the checkout path if necessary.
    if not self.m.path.exists(self.m.vars.checkout_root):
      self.m.file.ensure_directory('makedirs checkout_path',
                                   self.m.vars.checkout_root)

    # Initial cleanup.
    gclient_cfg = self.m.gclient.make_config(**cfg_kwargs)
    main_repo = self.m.properties['repository']
    if self.m.vars.need_pdfium_checkout:
      main_repo = 'https://pdfium.googlesource.com/pdfium.git'
    if self.m.vars.need_flutter_checkout:
      main_repo = 'https://github.com/flutter/engine.git'
    main_name = self.m.path.basename(main_repo)
    if main_name.endswith('.git'):
      main_name = main_name[:-len('.git')]
      # Special case for flutter because it seems to need a very specific
      # directory structure to successfully build.
      if self.m.vars.need_flutter_checkout and main_name == 'engine':
        main_name = 'src/flutter'
    main = gclient_cfg.solutions.add()
    main.name = main_name
    main.managed = False
    main.url = main_repo
    main.revision = self.m.properties.get('revision') or 'origin/master'
    m = gclient_cfg.got_revision_mapping
    m[main_name] = 'got_revision'
    patch_root = main_name
    patch_repo = main.url
    if self.m.properties.get('patch_repo'):
      patch_repo = self.m.properties['patch_repo']
      patch_root = patch_repo.split('/')[-1]
      if patch_root.endswith('.git'):
        patch_root = patch_root[:-4]

    if self.m.vars.need_pdfium_checkout:
      # Skia is a DEP of PDFium; the 'revision' property is a Skia revision, and
      # any patch should be applied to Skia, not PDFium.
      main.revision = 'origin/master'
      main.managed = True
      m[main_name] = 'got_%s_revision' % main_name

      skia_dep_path = 'pdfium/third_party/skia'
      gclient_cfg.patch_projects['skia'] = (skia_dep_path, 'HEAD')
      gclient_cfg.revisions[skia_dep_path] = self.m.properties['revision']
      m[skia_dep_path] = 'got_revision'
      patch_repo = 'https://skia.googlesource.com/skia.git'
      patch_root = skia_dep_path

    if self.m.vars.need_flutter_checkout:
      # Skia is a DEP of Flutter; the 'revision' property is a Skia revision,
      # and any patch should be applied to Skia, not Flutter.
      main.revision = 'origin/master'
      main.managed = True
      m[main_name] = 'got_flutter_revision'
      if 'Android' in self.m.vars.extra_tokens:
        gclient_cfg.target_os.add('android')

      skia_dep_path = 'src/third_party/skia'
      gclient_cfg.patch_projects['skia'] = (skia_dep_path, 'HEAD')
      gclient_cfg.revisions[skia_dep_path] = self.m.properties['revision']
      m[skia_dep_path] = 'got_revision'
      patch_repo = 'https://skia.googlesource.com/skia.git'
      patch_root = skia_dep_path

    # TODO(rmistry): Remove the below block after there is a solution for
    #                crbug.com/616443
    entries_file = self.m.vars.checkout_root.join('.gclient_entries')
    if self.m.path.exists(entries_file) or self._test_data.enabled:
      self.m.file.remove('remove %s' % entries_file,
                         entries_file)

    if self.m.vars.need_chromium_checkout:
      chromium = gclient_cfg.solutions.add()
      chromium.name = 'src'
      chromium.managed = False
      chromium.url = 'https://chromium.googlesource.com/chromium/src.git'
      chromium.revision = 'origin/lkcr'

    # Run bot_update.

    # TODO(borenet): Remove this hack after the transition to Kitchen:
    # https://bugs.chromium.org/p/skia/issues/detail?id=7050
    if self.m.vars.need_chromium_checkout:
      depot_tools = self.m.vars.checkout_root.join('depot_tools')
      res = depot_tools.join(
        'recipes', 'recipe_modules', 'bot_update', 'resources')
      self.m.git.checkout(
          'https://chromium.googlesource.com/chromium/tools/depot_tools.git',
          dir_path=depot_tools, ref='master')
      def resource(r):
        return res.join(r)
      self.m.bot_update.resource = resource

    # Hack the patch ref if necessary.
    if self.m.bot_update._issue and self.m.bot_update._patchset:
      self.m.bot_update._gerrit_ref = self.patch_ref(
          str(self.m.bot_update._issue), str(self.m.bot_update._patchset))
      self.m.bot_update._repository = patch_repo

    if not self.m.vars.is_trybot and is_parent_revision:
      main.revision = main.revision + '^'

    self.m.gclient.c = gclient_cfg
    with self.m.context(cwd=self.m.vars.checkout_root):
      update_step = self.m.bot_update.ensure_checkout(
          patch_root=patch_root,
          # The logic in ensure_checkout for this arg is fairly naive, so if
          # patch=False, we'll see "... (without patch)" in the step names, even
          # for non-trybot runs, which is misleading and confusing. Therefore,
          # always specify patch=True for non-trybot runs.
          patch=not (self.m.vars.is_trybot and is_parent_revision)
      )

    self.m.vars.got_revision = (
        update_step.presentation.properties['got_revision'])

    if self.m.vars.need_chromium_checkout:
      with self.m.context(cwd=self.m.vars.checkout_root,
                          env=self.m.vars.gclient_env):
        # TODO(borenet): Replace this hack with 'self.m.gclient.runhooks' after
        # the transition to Kitchen.
        self.m.run(self.m.step, 'gclient runhooks',
                   cmd=[depot_tools.join('gclient'), 'runhooks'])
