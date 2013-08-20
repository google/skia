#!/usr/bin/python

# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


""" Look through skia-autogen, searching for all checksums which should have
corresponding files in Google Storage, and verify that those files exist. """


import json
import posixpath
import re
import subprocess
import sys


# TODO(borenet): Replace some/all of these with constants from gm/gm_json.py
AUTOGEN_URL = 'http://skia-autogen.googlecode.com/svn/gm-actual'
GS_URL = 'gs://chromium-skia-gm/gm'
TEST_NAME_PATTERN = re.compile('(\S+)_(\S+).png')


def FileNameToGSURL(filename, hash_type, hash_value):
  """ Convert a file name given in a checksum file to the URL of the
  corresponding image file in Google Storage.

  filename: string; the file name to convert. Takes the form specified by
      TEST_NAME_PATTERN.
  hash_type: string; the type of the checksum.
  hash_value: string; the checksum itself.
  """
  test_name = TEST_NAME_PATTERN.match(filename).group(1)
  if not test_name:
    raise Exception('Invalid test name for file: %s' % filename)
  return '%s/%s/%s/%s.png' % (GS_URL, hash_type, test_name, hash_value)


def FindURLSInJSON(json_file, gs_urls):
  """ Extract Google Storage URLs from a JSON file in svn, adding them to the
  gs_urls dictionary.

  json_file: string; URL of the JSON file.
  gs_urls: dict; stores Google Storage URLs as keys and lists of the JSON files
      which reference them.

  Example gs_urls:
  { 'gs://chromium-skia-gm/gm/sometest/12345.png': [
      'http://skia-autogen.googlecode.com/svn/gm-actual/Test-Mac10.6-MacMini4.1-GeForce320M-x86-Debug/actual-results.json',
      'http://skia-autogen.googlecode.com/svn/gm-actual/Test-Mac10.8-MacMini4.1-GeForce320M-x86-Debug/actual-results.json',
    ]
  }
  """
  output = subprocess.check_output(['svn', 'cat', json_file])
  json_content = json.loads(output)
  for dict_type in ['actual-results']:
    for result_type in json_content[dict_type]:
      if json_content[dict_type][result_type]:
        for result in json_content[dict_type][result_type].keys():
          hash_type, hash_value = json_content[dict_type][result_type][result]
          gs_url = FileNameToGSURL(result, hash_type, str(hash_value))
          if gs_urls.get(gs_url):
            gs_urls[gs_url].append(json_file)
          else:
            gs_urls[gs_url] = [json_file]


def _FindJSONFiles(url, json_files):
  """ Helper function for FindJsonFiles. Recursively explore the repository,
  adding JSON files to a list.

  url: string; URL of the repository (or subdirectory thereof) to explore.
  json_files: list to which JSON file urls will be added.
  """
  proc = subprocess.Popen(['svn', 'ls', url], stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT)
  if proc.wait() != 0:
    raise Exception('Failed to list svn directory.')
  output = proc.communicate()[0].splitlines()
  subdirs = []
  for item in output:
    if item.endswith(posixpath.sep):
      subdirs.append(item)
    elif item.endswith('.json'):
      json_files.append(posixpath.join(url, item))
    else:
      print 'Warning: ignoring %s' % posixpath.join(url, item)
  for subdir in subdirs:
    _FindJSONFiles(posixpath.join(url, subdir), json_files)


def FindJSONFiles(url):
  """ Recursively explore the given repository and return a list of the JSON
  files it contains.

  url: string; URL of the repository to explore.
  """
  print 'Searching for JSON files in %s' % url
  json_files = []
  _FindJSONFiles(url, json_files)
  return json_files


def FindURLs(url):
  """ Find Google Storage URLs inside of JSON files in the given repository.
  Returns a dictionary whose keys are Google Storage URLs and values are lists
  of the JSON files which reference them.

  url: string; URL of the repository to explore.

  Example output:
  { 'gs://chromium-skia-gm/gm/sometest/12345.png': [
      'http://skia-autogen.googlecode.com/svn/gm-actual/Test-Mac10.6-MacMini4.1-GeForce320M-x86-Debug/actual-results.json',
      'http://skia-autogen.googlecode.com/svn/gm-actual/Test-Mac10.8-MacMini4.1-GeForce320M-x86-Debug/actual-results.json',
    ]
  }
  """
  gs_urls = {}
  for json_file in FindJSONFiles(url):
    print 'Looking for checksums in %s' % json_file
    FindURLSInJSON(json_file, gs_urls)
  return gs_urls


def VerifyURL(url):
  """ Verify that the given URL exists.

  url: string; the Google Storage URL of the image file in question.
  """
  proc = subprocess.Popen(['gsutil', 'ls', url], stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT)
  if proc.wait() != 0:
    return False
  return True


def VerifyURLs(urls):
  """ Verify that each of the given URLs exists. Return a list of which URLs do
  not exist.
  
  urls: dictionary; URLs of the image files in question.
  """
  print 'Verifying that images exist for URLs...'
  missing = []
  for url in urls.iterkeys():
    if not VerifyURL(url):
      print 'Missing: %s, referenced by: \n  %s' % (url, '\n  '.join(urls[url]))
      missing.append(url)
  return missing


def Main():
  urls = FindURLs(AUTOGEN_URL)
  missing = VerifyURLs(urls)
  if missing:
    print 'Found %d Missing files.' % len(missing)
    return 1


if __name__ == '__main__':
  sys.exit(Main())
