# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


from recipe_engine import recipe_api
from recipe_engine import config_types


class CheckoutApi(recipe_api.RecipeApi):

  @property
  def default_checkout_root(self):
    """The default location for cached persistent checkouts."""
    return self.m.vars.cache_dir.join('work')

  def patch_ref(self, issue, patchset):
    """Build a ref for the given issue and patchset."""
    return 'refs/changes/%s/%s/%s' % (issue[-2:], issue, patchset)

  def git(self, checkout_root):
    """Run the steps to perform a pure-git checkout without DEPS."""
    skia_dir = checkout_root.join('skia')
    self.m.git.checkout(
        self.m.properties['repository'], dir_path=skia_dir,
        ref=self.m.properties['revision'], submodules=False)
    if self.m.vars.is_trybot:
      ref = self.patch_ref(str(self.m.vars.issue), str(self.m.vars.patchset))
      self.m.git('fetch', 'origin', ref)
      self.m.git('checkout', 'FETCH_HEAD')
      self.m.git('rebase', self.m.properties['revision'])
      return self.m.properties['revision']

  def bot_update(self, checkout_root, gclient_cache=None,
                 checkout_chromium=False, checkout_flutter=False,
                 extra_gclient_env=None, parent_rev=False,
                 flutter_android=False):
    """Run the steps to obtain a checkout using bot_update.

    Args:
      checkout_root: Root directory where the code will be synced.
      gclient_cache: Optional, directory of the gclient cache.
      checkout_chromium: If True, will check out chromium/src.git in addition
          to the primary repo.
      checkout_flutter: If True, will checkout flutter in addition to the
          primary repo.
      extra_gclient_env: Map of extra environment variable names to their values
          to supply while running gclient.
      parent_rev: If True, checks out the parent of the specified revision,
          rather than the revision itself, ie. HEAD^ for normal jobs and HEAD
          (no patch) for try jobs.
      flutter_android: Indicates that we're checking out flutter for Android.
    """
    if not gclient_cache:
      gclient_cache = self.m.vars.cache_dir.join('git')
    if not extra_gclient_env:
      extra_gclient_env = {}

    cfg_kwargs = {}

    # Use a persistent gclient cache for Swarming.
    cfg_kwargs['CACHE_DIR'] = gclient_cache

    # Create the checkout path if necessary.
    # TODO(borenet): 'makedirs checkout_root'
    self.m.file.ensure_directory('makedirs checkout_path', checkout_root)

    # Initial cleanup.
    gclient_cfg = self.m.gclient.make_config(**cfg_kwargs)

    main_repo = self.m.properties['repository']
    if checkout_flutter:
      main_repo = 'https://github.com/flutter/engine.git'
    main_name = self.m.path.basename(main_repo)
    if main_name.endswith('.git'):
      main_name = main_name[:-len('.git')]
      # Special case for flutter because it seems to need a very specific
      # directory structure to successfully build.
      if checkout_flutter and main_name == 'engine':
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

    if checkout_flutter:
      # Skia is a DEP of Flutter; the 'revision' property is a Skia revision,
      # and any patch should be applied to Skia, not Flutter.
      main.revision = 'origin/master'
      main.managed = True
      m[main_name] = 'got_flutter_revision'
      if flutter_android:
        gclient_cfg.target_os.add('android')

      skia_dep_path = 'src/third_party/skia'
      gclient_cfg.patch_projects['skia'] = (skia_dep_path, 'HEAD')
      gclient_cfg.revisions[skia_dep_path] = self.m.properties['revision']
      m[skia_dep_path] = 'got_revision'
      patch_repo = 'https://skia.googlesource.com/skia.git'
      patch_root = skia_dep_path

    if checkout_chromium:
      main.custom_vars['checkout_chromium'] = True
      extra_gclient_env['GYP_CHROMIUM_NO_ACTION'] = '0'

    # TODO(rmistry): Remove the below block after there is a solution for
    #                crbug.com/616443
    entries_file = checkout_root.join('.gclient_entries')
    if self.m.path.exists(entries_file) or self._test_data.enabled:
      self.m.file.remove('remove %s' % entries_file,
                         entries_file)

    # Run bot_update.
    if patch_repo != self.m.properties['repository']:
      # TODO(borenet): bot_update uses the 'repository' property to determine
      # which repo the patch should come from. This conflicts with our usage of
      # the same property to determine which root repo to check out, which may
      # not be the same as the repository the patch comes from, for which we use
      # the patch_repo property. Remove this hack by refactoring our checkout
      # code and properties to agree with bot_update.
      self.m.bot_update._repository = patch_repo

    if not self.m.vars.is_trybot and parent_rev:
      main.revision = main.revision + '^'

    self.m.gclient.c = gclient_cfg
    with self.m.context(cwd=checkout_root):
      update_step = self.m.bot_update.ensure_checkout(
          patch_root=patch_root,
          # The logic in ensure_checkout for this arg is fairly naive, so if
          # patch=False, we'll see "... (without patch)" in the step names, even
          # for non-trybot runs, which is misleading and confusing. Therefore,
          # always specify patch=True for non-trybot runs.
          patch=not (self.m.vars.is_trybot and parent_rev)
      )

    if checkout_chromium or checkout_flutter:
      gclient_env = {'DEPOT_TOOLS_UPDATE': '0'}
      if extra_gclient_env:
        gclient_env.update(extra_gclient_env)
      with self.m.context(cwd=checkout_root, env=gclient_env):
        self.m.gclient.runhooks()
    return update_step.presentation.properties['got_revision']
