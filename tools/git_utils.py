# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Module to host the ChangeGitBranch class and test_git_executable function.
"""

import os
import subprocess

import misc_utils


class ChangeGitBranch(object):
    """Class to manage git branches.

    This class allows one to create a new branch in a repository based
    off of a given commit, and restore the original tree state.

    Assumes current working directory is a git repository.

    Example:
        with ChangeGitBranch():
            edit_files(files)
            git_add(files)
            git_commit()
            git_format_patch('HEAD~')
        # At this point, the repository is returned to its original
        # state.

    Constructor Args:
        branch_name: (string) if not None, the name of the branch to
            use.  If None, then use a temporary branch that will be
            deleted.  If the branch already exists, then a different
            branch name will be created.  Use git_branch_name() to
            find the actual branch name used.
        upstream_branch: (string) if not None, the name of the branch or
            commit to branch from.  If None, then use origin/master
        verbose: (boolean) if true, makes debugging easier.

    Raises:
        OSError: the git executable disappeared.
        subprocess.CalledProcessError: git returned unexpected status.
        Exception: if the given branch name exists, or if the repository
            isn't clean on exit, or git can't be found.
    """
    # pylint: disable=I0011,R0903,R0902

    def __init__(self,
                 branch_name=None,
                 upstream_branch=None,
                 verbose=False):
        # pylint: disable=I0011,R0913
        if branch_name:
            self._branch_name = branch_name
            self._delete_branch = False
        else:
            self._branch_name = 'ChangeGitBranchTempBranch'
            self._delete_branch = True

        if upstream_branch:
            self._upstream_branch = upstream_branch
        else:
            self._upstream_branch = 'origin/master'

        self._git = git_executable()
        if not self._git:
            raise Exception('Git can\'t be found.')

        self._stash = None
        self._original_branch = None
        self._vsp = misc_utils.VerboseSubprocess(verbose)

    def _has_git_diff(self):
        """Return true iff repository has uncommited changes."""
        return bool(self._vsp.call([self._git, 'diff', '--quiet', 'HEAD']))

    def _branch_exists(self, branch):
        """Return true iff branch exists."""
        return 0 == self._vsp.call([self._git, 'show-ref', '--quiet', branch])

    def __enter__(self):
        git, vsp = self._git, self._vsp

        if self._branch_exists(self._branch_name):
            i, branch_name = 0, self._branch_name
            while self._branch_exists(branch_name):
                i += 1
                branch_name = '%s_%03d' % (self._branch_name, i)
            self._branch_name = branch_name

        self._stash = self._has_git_diff()
        if self._stash:
            vsp.check_call([git, 'stash', 'save'])
        self._original_branch = git_branch_name(vsp.verbose)
        vsp.check_call(
            [git, 'checkout', '-q', '-b',
             self._branch_name, self._upstream_branch])

    def __exit__(self, etype, value, traceback):
        git, vsp = self._git, self._vsp

        if self._has_git_diff():
            status = vsp.check_output([git, 'status', '-s'])
            raise Exception('git checkout not clean:\n%s' % status)
        vsp.check_call([git, 'checkout', '-q', self._original_branch])
        if self._stash:
            vsp.check_call([git, 'stash', 'pop'])
        if self._delete_branch:
            assert self._original_branch != self._branch_name
            vsp.check_call([git, 'branch', '-D', self._branch_name])


def git_branch_name(verbose=False):
    """Return a description of the current branch.

    Args:
        verbose: (boolean) makes debugging easier

    Returns:
        A string suitable for passing to `git checkout` later.
    """
    git = git_executable()
    vsp = misc_utils.VerboseSubprocess(verbose)
    try:
        full_branch = vsp.strip_output([git, 'symbolic-ref', 'HEAD'])
        return full_branch.split('/')[-1]
    except (subprocess.CalledProcessError,):
        # "fatal: ref HEAD is not a symbolic ref"
        return vsp.strip_output([git, 'rev-parse', 'HEAD'])


def test_git_executable(git):
    """Test the git executable.

    Args:
        git: git executable path.
    Returns:
        True if test is successful.
    """
    with open(os.devnull, 'w') as devnull:
        try:
            subprocess.call([git, '--version'], stdout=devnull)
        except (OSError,):
            return False
    return True


def git_executable():
    """Find the git executable.

    If the GIT_EXECUTABLE environment variable is set, that will
    override whatever is found in the PATH.

    If no suitable executable is found, return None

    Returns:
        A string suiable for passing to subprocess functions, or None.
    """
    env_git = os.environ.get('GIT_EXECUTABLE')
    if env_git and test_git_executable(env_git):
        return env_git
    for git in ('git', 'git.exe', 'git.bat'):
        if test_git_executable(git):
            return git
    return None

