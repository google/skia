#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Utilities for working with URLs.

TODO(epoger): move this into tools/utils for broader use?
"""

# System-level imports
import contextlib
import os
import shutil
import urllib
import urlparse


def create_filepath_url(filepath):
  """ Returns a file:/// URL pointing at the given filepath on local disk.

  Args:
    filepath: string; path to a file on local disk (may be absolute or relative,
        and the file does not need to exist)

  Returns:
    A file:/// URL pointing at the file.  Regardless of whether filepath was
        specified as a relative or absolute path, the URL will contain an
        absolute path to the file.

  Raises:
    An Exception, if filepath is already a URL.
  """
  if urlparse.urlparse(filepath).scheme:
    raise Exception('"%s" is already a URL' % filepath)
  return urlparse.urljoin(
      'file:', urllib.pathname2url(os.path.abspath(filepath)))


def copy_contents(source_url, dest_path, create_subdirs_if_needed=False):
  """ Copies the full contents of the URL 'source_url' into
  filepath 'dest_path'.

  Args:
    source_url: string; complete URL to read from
    dest_path: string; complete filepath to write to (may be absolute or
        relative)
    create_subdirs_if_needed: boolean; whether to create subdirectories as
        needed to create dest_path

  Raises:
    Some subclass of Exception if unable to read source_url or write dest_path.
  """
  if create_subdirs_if_needed:
    dest_dir = os.path.dirname(dest_path)
    if not os.path.exists(dest_dir):
      os.makedirs(dest_dir)
  with contextlib.closing(urllib.urlopen(source_url)) as source_handle:
    with open(dest_path, 'wb') as dest_handle:
      shutil.copyfileobj(fsrc=source_handle, fdst=dest_handle)
