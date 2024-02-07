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

  def assert_git_is_from_cipd(self):
    """Fail if git is not obtained from CIPD."""
    script = self.resource('assert_git_cipd.py')
    self.m.run(
        self.m.step, 'Assert that Git is from CIPD', cmd=['python3', script])

  def git(self, checkout_root):
    """Run the steps to perform a pure-git checkout without DEPS."""
    self.assert_git_is_from_cipd()
    skia_dir = checkout_root.join('skia')
    self.m.git.checkout(
        self.m.properties['repository'], dir_path=skia_dir,
        ref=self.m.properties['revision'], submodules=False)
    if self.m.vars.is_trybot:
      self.m.git('fetch', 'origin', self.m.properties['patch_ref'])
      self.m.git('checkout', 'FETCH_HEAD')
      self.m.git('rebase', self.m.properties['revision'])
      return self.m.properties['revision']

  def bot_update(self, checkout_root, gclient_cache=None,
                 skip_patch=False, override_revision=None):
    """Run the steps to obtain a checkout using bot_update.

    Args:
      checkout_root: Root directory where the code will be synced.
      gclient_cache: Optional, directory of the gclient cache.
      skip_patch: Ignore changelist/patchset when syncing the Skia repo.
    """
    self.assert_git_is_from_cipd()
    if not gclient_cache:
      gclient_cache = self.m.vars.cache_dir.join('git')

    cfg_kwargs = {}

    # Use a persistent gclient cache for Swarming.
    cfg_kwargs['CACHE_DIR'] = gclient_cache

    # Create the checkout path if necessary.
    # TODO(borenet): 'makedirs checkout_root'
    self.m.file.ensure_directory('makedirs checkout_path', checkout_root)

    # Initial cleanup.
    gclient_cfg = self.m.gclient.make_config(**cfg_kwargs)

    main_repo = self.m.properties['repository']
    main_name = self.m.path.basename(main_repo)
    if main_name.endswith('.git'):
      main_name = main_name[:-len('.git')]
    main = gclient_cfg.solutions.add()
    main.name = main_name
    main.managed = False
    main.url = main_repo
    main.revision = (override_revision or
      self.m.properties.get('revision') or 'origin/main')
    m = gclient_cfg.got_revision_mapping
    m[main_name] = 'got_revision'
    patch_root = main_name
    patch_repo = main.url
    if self.m.properties.get('patch_repo'):
      patch_repo = self.m.properties['patch_repo']
      patch_root = patch_repo.split('/')[-1]
      if patch_root.endswith('.git'):
        patch_root = patch_root[:-4]

    # TODO(rmistry): Remove the below block after there is a solution for
    #                crbug.com/616443
    entries_file = checkout_root.join('.gclient_entries')
    if self.m.path.exists(entries_file) or self._test_data.enabled:
      self.m.file.remove('remove %s' % entries_file,
                         entries_file)

    # Run bot_update.
    patch_refs = None
    patch_ref = self.m.properties.get('patch_ref')
    if patch_ref and not skip_patch:
      patch_refs = ['%s@%s:%s' % (self.m.properties['patch_repo'],
                                  self.m.properties['revision'],
                                  patch_ref)]

    self.m.gclient.c = gclient_cfg
    with self.m.context(cwd=checkout_root):
      # https://chromium.googlesource.com/chromium/tools/depot_tools.git/+/dca14bc463857bd2a0fee59c86ffa289b535d5d3/recipes/recipe_modules/bot_update/api.py#105
      update_step = self.m.bot_update.ensure_checkout(
          patch_root=patch_root,
          # The logic in ensure_checkout for this arg is fairly naive, so if
          # patch=False, we'll see "... (without patch)" in the step names, even
          # for non-trybot runs, which is misleading and confusing. Therefore,
          # always specify patch=True.
          patch=True,
          patch_refs=patch_refs,
          # Download the patches of all changes with the same Gerrit topic.
          # For context see go/sk-topics.
          download_topics=True,
      )

    return update_step.presentation.properties['got_revision']
