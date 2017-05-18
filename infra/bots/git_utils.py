#!/usr/bin/env python
# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""This module contains functions for using git."""

import re
import shutil
import subprocess
import tempfile

import utils


class GitLocalConfig(object):
  """Class to manage local git configs."""
  def __init__(self, config_dict):
    self._config_dict = config_dict
    self._previous_values = {}

  def __enter__(self):
    for k, v in self._config_dict.iteritems():
      try:
        prev = subprocess.check_output(['git', 'config', '--local', k]).rstrip()
        if prev:
          self._previous_values[k] = prev
      except subprocess.CalledProcessError:
        # We are probably here because the key did not exist in the config.
        pass
      subprocess.check_call(['git', 'config', '--local', k, v])

  def __exit__(self, exc_type, _value, _traceback):
    for k in self._config_dict:
      if self._previous_values.get(k):
        subprocess.check_call(
            ['git', 'config', '--local', k, self._previous_values[k]])
      else:
        subprocess.check_call(['git', 'config', '--local', '--unset', k])


class GitBranch(object):
  """Class to manage git branches.

  This class allows one to create a new branch in a repository to make changes,
  then it commits the changes, switches to master branch, and deletes the
  created temporary branch upon exit.
  """
  def __init__(self, branch_name, commit_msg, upload=True, commit_queue=False,
               delete_when_finished=True):
    self._branch_name = branch_name
    self._commit_msg = commit_msg
    self._upload = upload
    self._commit_queue = commit_queue
    self._patch_set = 0
    self._delete_when_finished = delete_when_finished

  def __enter__(self):
    subprocess.check_call(['git', 'reset', '--hard', 'HEAD'])
    subprocess.check_call(['git', 'checkout', 'master'])
    if self._branch_name in subprocess.check_output(['git', 'branch']).split():
      subprocess.check_call(['git', 'branch', '-D', self._branch_name])
    subprocess.check_call(['git', 'checkout', '-b', self._branch_name,
                           '-t', 'origin/master'])
    return self

  def commit_and_upload(self, use_commit_queue=False):
    """Commit all changes and upload a CL, returning the issue URL."""
    subprocess.check_call(['git', 'commit', '-a', '-m', self._commit_msg])
    upload_cmd = ['git', 'cl', 'upload', '-f', '--bypass-hooks',
                  '--bypass-watchlists']
    self._patch_set += 1
    if self._patch_set > 1:
      upload_cmd.extend(['-t', 'Patch set %d' % self._patch_set])
    if use_commit_queue:
      upload_cmd.append('--use-commit-queue')
    subprocess.check_call(upload_cmd)
    output = subprocess.check_output(['git', 'cl', 'issue']).rstrip()
    return re.match('^Issue number: (?P<issue>\d+) \((?P<issue_url>.+)\)$',
                    output).group('issue_url')

  def __exit__(self, exc_type, _value, _traceback):
    if self._upload:
      # Only upload if no error occurred.
      try:
        if exc_type is None:
          self.commit_and_upload(use_commit_queue=self._commit_queue)
      finally:
        subprocess.check_call(['git', 'checkout', 'master'])
        if self._delete_when_finished:
          subprocess.check_call(['git', 'branch', '-D', self._branch_name])


class NewGitCheckout(utils.tmp_dir):
  """Creates a new local checkout of a Git repository."""

  def __init__(self, repository, commit='HEAD'):
    """Set parameters for this local copy of a Git repository.

    Because this is a new checkout, rather than a reference to an existing
    checkout on disk, it is safe to assume that the calling thread is the
    only thread manipulating the checkout.

    You must use the 'with' statement to create this object:

    with NewGitCheckout(*args) as checkout:
      # use checkout instance
    # the checkout is automatically cleaned up here

    Args:
      repository: URL of the remote repository (e.g.,
          'https://skia.googlesource.com/common') or path to a local repository
          (e.g., '/path/to/repo/.git') to check out a copy of
      commit: commit hash, branch, or tag within refspec, indicating what point
          to update the local checkout to
    """
    super(NewGitCheckout, self).__init__()
    self._repository = repository
    self._commit = commit

  @property
  def root(self):
    """Returns the root directory containing the checked-out files."""
    return self.name

  def __enter__(self):
    """Check out a new local copy of the repository.

    Uses the parameters that were passed into the constructor.
    """
    super(NewGitCheckout, self).__enter__()
    subprocess.check_output(args=['git', 'clone', self._repository, self.root])
    return self
