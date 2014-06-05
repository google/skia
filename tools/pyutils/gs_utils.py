#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Utilities for accessing Google Cloud Storage.

TODO(epoger): move this into tools/utils for broader use?
"""

# System-level imports
import os
import posixpath
import sys
try:
  from apiclient.discovery import build as build_service
except ImportError:
  print ('Missing google-api-python-client.  Please install it; directions '
         'can be found at https://developers.google.com/api-client-library/'
         'python/start/installation')
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
