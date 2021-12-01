#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from __future__ import print_function
import datetime
import errno
import os
import shutil
import sys
import subprocess
import tempfile
import time
import uuid


SKIA_REPO = 'https://skia.googlesource.com/skia.git'

GCLIENT = 'gclient.bat' if sys.platform == 'win32' else 'gclient'
WHICH = 'where' if sys.platform == 'win32' else 'which'
GIT = subprocess.check_output([WHICH, 'git']).decode('utf-8').splitlines()[0]

class print_timings(object):
  def __init__(self):
    self._start = None

  def __enter__(self):
    self._start = datetime.datetime.utcnow()
    print('Task started at %s GMT' % str(self._start))

  def __exit__(self, t, v, tb):
    finish = datetime.datetime.utcnow()
    duration = (finish-self._start).total_seconds()
    print('Task finished at %s GMT (%f seconds)' % (str(finish), duration))


class tmp_dir(object):
  """Helper class used for creating a temporary directory and working in it."""
  def __init__(self):
    self._orig_dir = None
    self._tmp_dir = None

  def __enter__(self):
    self._orig_dir = os.getcwd()
    self._tmp_dir = tempfile.mkdtemp()
    os.chdir(self._tmp_dir)
    return self

  def __exit__(self, t, v, tb):
    os.chdir(self._orig_dir)
    RemoveDirectory(self._tmp_dir)

  @property
  def name(self):
    return self._tmp_dir


class chdir(object):
  """Helper class used for changing into and out of a directory."""
  def __init__(self, d):
    self._dir = d
    self._orig_dir = None

  def __enter__(self):
    self._orig_dir = os.getcwd()
    os.chdir(self._dir)
    return self

  def __exit__(self, t, v, tb):
    os.chdir(self._orig_dir)


def git_clone(repo_url, dest_dir):
  """Clone the given repo into the given destination directory."""
  subprocess.check_call([GIT, 'clone', repo_url, dest_dir])


class git_branch(object):
  """Check out a temporary git branch.

  On exit, deletes the branch and attempts to restore the original state.
  """
  def __init__(self):
    self._branch = None
    self._orig_branch = None
    self._stashed = False

  def __enter__(self):
    output = subprocess.check_output([GIT, 'stash']).decode('utf-8')
    self._stashed = 'No local changes' not in output

    # Get the original branch name or commit hash.
    self._orig_branch = subprocess.check_output([
        GIT, 'rev-parse', '--abbrev-ref', 'HEAD']).decode('utf-8').rstrip()
    if self._orig_branch == 'HEAD':
      self._orig_branch = subprocess.check_output([
          GIT, 'rev-parse', 'HEAD']).decode('utf-8').rstrip()

    # Check out a new branch, based at updated origin/main.
    subprocess.check_call([GIT, 'fetch', 'origin'])
    self._branch = '_tmp_%s' % uuid.uuid4()
    subprocess.check_call([GIT, 'checkout', '-b', self._branch,
                           '-t', 'origin/main'])
    return self

  def __exit__(self, exc_type, _value, _traceback):
    subprocess.check_call([GIT, 'reset', '--hard', 'HEAD'])
    subprocess.check_call([GIT, 'checkout', self._orig_branch])
    if self._stashed:
      subprocess.check_call([GIT, 'stash', 'pop'])
    subprocess.check_call([GIT, 'branch', '-D', self._branch])


def RemoveDirectory(*path):
  """Recursively removes a directory, even if it's marked read-only.

  This was copied from:
  https://chromium.googlesource.com/chromium/tools/build/+/f3e7ff03613cd59a463b2ccc49773c3813e77404/scripts/common/chromium_utils.py#491

  Remove the directory located at *path, if it exists.

  shutil.rmtree() doesn't work on Windows if any of the files or directories
  are read-only, which svn repositories and some .svn files are.  We need to
  be able to force the files to be writable (i.e., deletable) as we traverse
  the tree.

  Even with all this, Windows still sometimes fails to delete a file, citing
  a permission error (maybe something to do with antivirus scans or disk
  indexing).  The best suggestion any of the user forums had was to wait a
  bit and try again, so we do that too.  It's hand-waving, but sometimes it
  works. :/
  """
  file_path = os.path.join(*path)
  if not os.path.exists(file_path):
    return

  if sys.platform == 'win32':
    # Give up and use cmd.exe's rd command.
    file_path = os.path.normcase(file_path)
    for _ in range(3):
      print('RemoveDirectory running %s' % (' '.join(
          ['cmd.exe', '/c', 'rd', '/q', '/s', file_path])))
      if not subprocess.call(['cmd.exe', '/c', 'rd', '/q', '/s', file_path]):
        break
      print('  Failed')
      time.sleep(3)
    return

  def RemoveWithRetry_non_win(rmfunc, path):
    if os.path.islink(path):
      return os.remove(path)
    else:
      return rmfunc(path)

  remove_with_retry = RemoveWithRetry_non_win

  def RmTreeOnError(function, path, excinfo):
    r"""This works around a problem whereby python 2.x on Windows has no ability
    to check for symbolic links.  os.path.islink always returns False.  But
    shutil.rmtree will fail if invoked on a symbolic link whose target was
    deleted before the link.  E.g., reproduce like this:
    > mkdir test
    > mkdir test\1
    > mklink /D test\current test\1
    > python -c "import chromium_utils; chromium_utils.RemoveDirectory('test')"
    To avoid this issue, we pass this error-handling function to rmtree.  If
    we see the exact sort of failure, we ignore it.  All other failures we re-
    raise.
    """

    exception_type = excinfo[0]
    exception_value = excinfo[1]
    # If shutil.rmtree encounters a symbolic link on Windows, os.listdir will
    # fail with a WindowsError exception with an ENOENT errno (i.e., file not
    # found).  We'll ignore that error.  Note that WindowsError is not defined
    # for non-Windows platforms, so we use OSError (of which it is a subclass)
    # to avoid lint complaints about an undefined global on non-Windows
    # platforms.
    if (function is os.listdir) and issubclass(exception_type, OSError):
      if exception_value.errno == errno.ENOENT:
        # File does not exist, and we're trying to delete, so we can ignore the
        # failure.
        print('WARNING:  Failed to list %s during rmtree.  Ignoring.\n' % path)
      else:
        raise
    else:
      raise

  for root, dirs, files in os.walk(file_path, topdown=False):
    # For POSIX:  making the directory writable guarantees removability.
    # Windows will ignore the non-read-only bits in the chmod value.
    os.chmod(root, 0o770)
    for name in files:
      remove_with_retry(os.remove, os.path.join(root, name))
    for name in dirs:
      remove_with_retry(lambda p: shutil.rmtree(p, onerror=RmTreeOnError),
                        os.path.join(root, name))

  remove_with_retry(os.rmdir, file_path)
