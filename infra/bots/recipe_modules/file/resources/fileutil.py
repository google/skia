#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# TODO(borenet): This module belongs in the recipe engine. Remove it from this
# repo once it has been moved.


"""Utility exporting basic filesystem operations.

This file was cut from "scripts/common/chromium_utils.py" at:
91310531c31fa645256b4fb5d44b460c42b3e151
"""

import argparse
import errno
import fnmatch
import os
import shutil
import subprocess
import sys
import time


def _LocateFiles(pattern, root):
  """Yeilds files matching pattern found in root and its subdirectories.

  An exception is thrown if root doesn't exist."""
  for path, _, files in os.walk(os.path.abspath(root)):
    for filename in fnmatch.filter(files, pattern):
      yield os.path.join(path, filename)


def _RemoveFilesWildcards(file_wildcard, root):
  """Removes files matching 'file_wildcard' in root and its subdirectories, if
  any exists.

  An exception is thrown if root doesn't exist."""
  for item in _LocateFiles(file_wildcard, root):
    try:
      os.remove(item)
    except OSError, e:
      if e.errno != errno.ENOENT:
        raise


def _RemoveContents(path):
  if os.path.exists(path):
    for p in (os.path.join(path, x) for x in os.listdir(path)):
      if os.path.isdir(p):
        _RemoveDirectory(p)
      else:
        os.unlink(p)


def _RemoveDirectory(*path):
  """Recursively removes a directory, even if it's marked read-only.

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
    for _ in xrange(3):
      print 'RemoveDirectory running %s' % (' '.join(
          ['cmd.exe', '/c', 'rd', '/q', '/s', file_path]))
      if not subprocess.call(['cmd.exe', '/c', 'rd', '/q', '/s', file_path]):
        break
      print '  Failed'
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
        print 'WARNING:  Failed to list %s during rmtree.  Ignoring.\n' % path
      else:
        raise
    else:
      raise

  for root, dirs, files in os.walk(file_path, topdown=False):
    # For POSIX:  making the directory writable guarantees removability.
    # Windows will ignore the non-read-only bits in the chmod value.
    os.chmod(root, 0770)
    for name in files:
      remove_with_retry(os.remove, os.path.join(root, name))
    for name in dirs:
      remove_with_retry(lambda p: shutil.rmtree(p, onerror=RmTreeOnError),
                        os.path.join(root, name))

  remove_with_retry(os.rmdir, file_path)


def main(args):
  parser = argparse.ArgumentParser()

  subparsers = parser.add_subparsers()

  # Subcommand: rmtree
  subparser = subparsers.add_parser('rmtree',
      help='Recursively remove a directory.')
  subparser.add_argument('path', nargs='+', help='A path to remove.')
  subparser.set_defaults(func=lambda opts: _RemoveDirectory(*opts.path))

  # Subcommand: rmcontents
  subparser = subparsers.add_parser('rmcontents',
      help='Recursively remove the contents of a directory.')
  subparser.add_argument('path', help='The target directory.')
  subparser.set_defaults(func=lambda opts: _RemoveContents(opts.path))

  # Subcommand: rmwildcard
  subparser = subparsers.add_parser('rmwildcard',
      help='Recursively remove the contents of a directory.')
  subparser.add_argument('root', help='The directory to search through.')
  subparser.add_argument('wildcard', help='The wildcard expression to remove.')
  subparser.set_defaults(func=lambda opts:
      _RemoveFilesWildcards(opts.wildcard, opts.root))

  # Parse arguments.
  opts = parser.parse_args(args)
  opts.func(opts)


if __name__ == '__main__':
  main(sys.argv[1:])
