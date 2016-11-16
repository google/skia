#!/usr/bin/env python
# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""This module contains functions for using git."""

import os
import re
import shell_utils
import shutil
import subprocess
import tempfile


def _FindGit():
  """Find the git executable.

  Returns:
      A string suitable for passing to subprocess functions, or None.
  """
  def test_git_executable(git):
    """Test the git executable.

    Args:
        git: git executable path.
    Returns:
        True if test is successful.
    """
    try:
      shell_utils.run([git, '--version'], echo=False)
      return True
    except (OSError,):
      return False

  for git in ('git', 'git.exe', 'git.bat'):
    if test_git_executable(git):
      return git
  return None


GIT = _FindGit()


def Add(addition):
  """Run 'git add <addition>'"""
  shell_utils.run([GIT, 'add', addition])


def AIsAncestorOfB(a, b):
  """Return true if a is an ancestor of b."""
  return shell_utils.run([GIT, 'merge-base', a, b]).rstrip() == FullHash(a)


def FullHash(commit):
  """Return full hash of specified commit."""
  return shell_utils.run([GIT, 'rev-parse', '--verify', commit]).rstrip()


def IsMerge(commit):
  """Return True if the commit is a merge, False otherwise."""
  rev_parse = shell_utils.run([GIT, 'rev-parse', commit, '--max-count=1',
                               '--no-merges'])
  last_non_merge = rev_parse.split('\n')[0]
  # Get full hash since that is what was returned by rev-parse.
  return FullHash(commit) != last_non_merge


def MergeAbort():
  """Abort in process merge."""
  shell_utils.run([GIT, 'merge', '--abort'])


def ShortHash(commit):
  """Return short hash of the specified commit."""
  return shell_utils.run([GIT, 'show', commit, '--format=%h', '-s']).rstrip()


def Fetch(remote=None):
  """Run "git fetch". """
  cmd = [GIT, 'fetch']
  if remote:
    cmd.append(remote)
  shell_utils.run(cmd)


def GetRemoteMasterHash(git_url):
  return shell_utils.run([GIT, 'ls-remote', git_url, '--verify',
                          'refs/heads/master']).rstrip()


def GetCurrentBranch():
  return shell_utils.run([GIT, 'rev-parse', '--abbrev-ref', 'HEAD']).rstrip()


# TODO(rmistry): Restore all old config values below.
class GitLocalConfig(object):
  """Class to manage local git configs."""
  def __init__(self, config_dict):
    self._config_dict = config_dict

  def __enter__(self):
    for k, v in self._config_dict.iteritems():
      subprocess.check_call(['git', 'config', '--local', k, v])

  def __exit__(self, exc_type, _value, _traceback):
    for k in self._config_dict:
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
    shell_utils.run([GIT, 'reset', '--hard', 'HEAD'])
    shell_utils.run([GIT, 'checkout', 'master'])
    if self._branch_name in shell_utils.run([GIT, 'branch']):
      shell_utils.run([GIT, 'branch', '-D', self._branch_name])
    shell_utils.run([GIT, 'checkout', '-b', self._branch_name,
                     '-t', 'origin/master'])
    return self

  def commit_and_upload(self, use_commit_queue=False):
    """Commit all changes and upload a CL, returning the issue URL."""
    try:
      shell_utils.run([GIT, 'commit', '-a', '-m', self._commit_msg])
    except shell_utils.CommandFailedException as e:
      if not 'nothing to commit' in e.output:
        raise
    upload_cmd = [GIT, 'cl', 'upload', '-f', '--bypass-hooks',
                  '--bypass-watchlists']
    self._patch_set += 1
    if self._patch_set > 1:
      upload_cmd.extend(['-t', 'Patch set %d' % self._patch_set])
    if use_commit_queue:
      upload_cmd.append('--use-commit-queue')
    shell_utils.run(upload_cmd)
    output = shell_utils.run([GIT, 'cl', 'issue']).rstrip()
    return re.match('^Issue number: (?P<issue>\d+) \((?P<issue_url>.+)\)$',
                    output).group('issue_url')

  def __exit__(self, exc_type, _value, _traceback):
    if self._upload:
      # Only upload if no error occurred.
      try:
        if exc_type is None:
          self.commit_and_upload(use_commit_queue=self._commit_queue)
      finally:
        shell_utils.run([GIT, 'checkout', 'master'])
        if self._delete_when_finished:
          shell_utils.run([GIT, 'branch', '-D', self._branch_name])


class NewGitCheckout(object):
  """Creates a new local checkout of a Git repository."""

  def __init__(self, repository, refspec=None, commit='HEAD',
               subdir=None, containing_dir=None):
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
      refspec: which refs (e.g., a branch name) to fetch from the repository;
          if None, git-fetch will choose the default refs to fetch
      commit: commit hash, branch, or tag within refspec, indicating what point
          to update the local checkout to
      subdir: if specified, the caller only wants access to files within this
          subdir in the repository.
          For now, we check out the entire repository regardless of this param,
          and just hide the rest of the repository; but later on we may
          optimize performance by only checking out this part of the repo.
      containing_dir: if specified, the new checkout will be created somewhere
          within this directory; otherwise, a system-dependent default location
          will be used, as determined by tempfile.mkdtemp()
    """
    self._repository = repository
    self._refspec = refspec
    self._commit = commit
    self._subdir = subdir
    self._containing_dir = containing_dir
    self._git_root = None
    self._file_root = None


  @property
  def root(self):
    """Returns the root directory containing the checked-out files.

    If you specified the subdir parameter in the constructor, this directory
    will point at just the subdir you requested.
    """
    return self._file_root

  def commithash(self):
    """Returns the commithash of the local checkout."""
    return self._run_in_git_root(
        args=[GIT, 'rev-parse', 'HEAD']).strip()

  def __enter__(self):
    """Check out a new local copy of the repository.

    Uses the parameters that were passed into the constructor.
    """
    # _git_root points to the tree holding the git checkout in its entirety;
    # _file_root points to the files the caller wants to look at
    self._git_root = tempfile.mkdtemp(dir=self._containing_dir)
    if self._subdir:
      self._file_root = os.path.join(self._git_root, self._subdir)
    else:
      self._file_root = self._git_root

    local_branch_name = 'local'
    self._run_in_git_root(args=[GIT, 'init'])
    fetch_cmd = [GIT, 'fetch', self._repository]
    if self._refspec:
      fetch_cmd.append(self._refspec)
    self._run_in_git_root(args=fetch_cmd)
    self._run_in_git_root(args=[GIT, 'merge', 'FETCH_HEAD'])
    self._run_in_git_root(args=[GIT, 'branch', local_branch_name, self._commit])
    self._run_in_git_root(args=[GIT, 'checkout', local_branch_name])

    return self

  # pylint: disable=W0622
  def __exit__(self, type, value, traceback):
    shutil.rmtree(self._git_root)

  def _run_in_git_root(self, args):
    """Run an external command with cwd set to self._git_root.

    Returns the command's output as a byte string.

    Raises an Exception if the command fails.
    """
    return subprocess.check_output(args=args, cwd=self._git_root)
