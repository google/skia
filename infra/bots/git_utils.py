#!/usr/bin/env python
# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""This module contains functions for using git."""

import re
import subprocess


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
