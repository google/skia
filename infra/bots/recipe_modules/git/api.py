# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Wrapper for Git which checks the validity of bot credentials."""


from recipe_engine import recipe_api


class GitApi(recipe_api.RecipeApi):
  def __init__(self, *args, **kwargs):
    super(GitApi, self).__init__(*args, **kwargs)
    self._checked = False


  def _check(self):
    """Check credentials."""
    if self._checked:
      return

    # This should really go in __init__, but self.m.properties hasn't been
    # loaded at that point.
    self._git = 'git'
    if 'Win' in self.m.properties.get('buildername', ''):
      self._git = 'git.bat'

    # Verify that the credential files are present.
    self.m.python.inline('Check git creds', program='''
import os
import sys

gitconfig = os.path.join(os.path.expanduser('~'), '.gitconfig')
netrc_base = '.netrc'
if sys.platform.startswith('win'):
  netrc_base = '_netrc'
netrc = os.path.join(os.path.expanduser('~'), netrc_base)

has_gitconfig = os.path.isfile(gitconfig)
has_netrc = os.path.isfile(netrc)

if not has_gitconfig and not has_netrc:
  raise Exception('Missing %s and %s' % (gitconfig, netrc))
if not has_gitconfig:
  raise Exception('Missing %s' % gitconfig)
if not has_netrc:
  raise Exception('Missing %s' % netrc)
''')
    self._checked = True

  def __call__(self, *cmd):
    self._check()
    name = 'git %s' % cmd[0]
    self.m.run(self.m.step, name, cmd=[self._git] + list(cmd), infra_step=True)
