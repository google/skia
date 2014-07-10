#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Utilities for accessing Google Cloud Storage.
"""

# System-level imports
import os
import posixpath
import sys

# Imports from third-party code
TRUNK_DIRECTORY = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
for import_subdir in ['google-api-python-client', 'httplib2', 'oauth2client',
                      'uritemplate-py']:
  import_dirpath = os.path.join(
      TRUNK_DIRECTORY, 'third_party', 'externals', import_subdir)
  if import_dirpath not in sys.path:
    # We need to insert at the beginning of the path, to make sure that our
    # imported versions are favored over others that might be in the path.
    # Also, the google-api-python-client checkout contains an empty
    # oauth2client directory, which will confuse things unless we insert
    # our checked-out oauth2client in front of it in the path.
    sys.path.insert(0, import_dirpath)
try:
  from googleapiclient.discovery import build as build_service
except ImportError:
  # TODO(epoger): We are moving toward not needing any dependencies to be
  # installed at a system level, but in the meanwhile, if developers run into
  # trouble they can install those system-level dependencies to get unblocked.
  print ('Missing dependencies of google-api-python-client.  Please install '
         'google-api-python-client to get those dependencies; directions '
         'can be found at https://developers.google.com/api-client-library/'
         'python/start/installation .  More details in http://skbug.com/2641 ')
  raise

# Local imports
import url_utils


def download_file(source_bucket, source_path, dest_path,
                  create_subdirs_if_needed=False):
  """ Downloads a single file from Google Cloud Storage to local disk.

  Args:
    source_bucket: GCS bucket to download the file from
    source_path: full path (Posix-style) within that bucket
    dest_path: full path (local-OS-style) on local disk to copy the file to
    create_subdirs_if_needed: boolean; whether to create subdirectories as
        needed to create dest_path
  """
  source_http_url = posixpath.join(
      'http://storage.googleapis.com', source_bucket, source_path)
  url_utils.copy_contents(source_url=source_http_url, dest_path=dest_path,
                          create_subdirs_if_needed=create_subdirs_if_needed)


def list_bucket_contents(bucket, subdir=None):
  """ Returns files in the Google Cloud Storage bucket as a (dirs, files) tuple.

  Uses the API documented at
  https://developers.google.com/storage/docs/json_api/v1/objects/list

  Args:
    bucket: name of the Google Storage bucket
    subdir: directory within the bucket to list, or None for root directory
  """
  # The GCS command relies on the subdir name (if any) ending with a slash.
  if subdir and not subdir.endswith('/'):
    subdir += '/'
  subdir_length = len(subdir) if subdir else 0

  storage = build_service('storage', 'v1')
  command = storage.objects().list(
      bucket=bucket, delimiter='/', fields='items(name),prefixes',
      prefix=subdir)
  results = command.execute()

  # The GCS command returned two subdicts:
  # prefixes: the full path of every directory within subdir, with trailing '/'
  # items: property dict for each file object within subdir
  #        (including 'name', which is full path of the object)
  dirs = []
  for dir_fullpath in results.get('prefixes', []):
    dir_basename = dir_fullpath[subdir_length:]
    dirs.append(dir_basename[:-1])  # strip trailing slash
  files = []
  for file_properties in results.get('items', []):
    file_fullpath = file_properties['name']
    file_basename = file_fullpath[subdir_length:]
    files.append(file_basename)
  return (dirs, files)
