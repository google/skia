#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Download actual GM results for a particular builder.
"""

# System-level imports
import contextlib
import optparse
import os
import posixpath
import re
import shutil
import sys
import urllib
import urllib2
import urlparse

# Imports from within Skia
#
# We need to add the 'gm' and 'tools' directories, so that we can import
# gm_json.py and buildbot_globals.py.
#
# Make sure that these dirs are in the PYTHONPATH, but add them at the *end*
# so any dirs that are already in the PYTHONPATH will be preferred.
#
# TODO(epoger): Is it OK for this to depend on the 'tools' dir, given that
# the tools dir is dependent on the 'gm' dir (to import gm_json.py)?
TRUNK_DIRECTORY = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
GM_DIRECTORY = os.path.join(TRUNK_DIRECTORY, 'gm')
TOOLS_DIRECTORY = os.path.join(TRUNK_DIRECTORY, 'tools')
if GM_DIRECTORY not in sys.path:
  sys.path.append(GM_DIRECTORY)
if TOOLS_DIRECTORY not in sys.path:
  sys.path.append(TOOLS_DIRECTORY)
import buildbot_globals
import gm_json

DEFAULT_ACTUALS_BASE_URL = posixpath.join(
    buildbot_globals.Get('autogen_svn_url'), 'gm-actual')
DEFAULT_JSON_FILENAME = 'actual-results.json'


class Download(object):

  def __init__(self, actuals_base_url=DEFAULT_ACTUALS_BASE_URL,
               json_filename=DEFAULT_JSON_FILENAME,
               gm_actuals_root_url=gm_json.GM_ACTUALS_ROOT_HTTP_URL):
    """
    Args:
      actuals_base_url: URL pointing at the root directory
          containing all actual-results.json files, e.g.,
          http://domain.name/path/to/dir  OR
          file:///absolute/path/to/localdir
      json_filename: The JSON filename to read from within each directory.
      gm_actuals_root_url: Base URL under which the actually-generated-by-bots
          GM images are stored.
    """
    self._actuals_base_url = actuals_base_url
    self._json_filename = json_filename
    self._gm_actuals_root_url = gm_actuals_root_url
    self._image_filename_re = re.compile(gm_json.IMAGE_FILENAME_PATTERN)

  def fetch(self, builder_name, dest_dir):
    """ Downloads actual GM results for a particular builder.

    Args:
      builder_name: which builder to download results of
      dest_dir: path to directory where the image files will be written;
                if the directory does not exist yet, it will be created

    TODO(epoger): Display progress info.  Right now, it can take a long time
    to download all of the results, and there is no indication of progress.

    TODO(epoger): Download multiple images in parallel to speed things up.
    """
    json_url = posixpath.join(self._actuals_base_url, builder_name,
                              self._json_filename)
    json_contents = urllib2.urlopen(json_url).read()
    results_dict = gm_json.LoadFromString(json_contents)

    actual_results_dict = results_dict[gm_json.JSONKEY_ACTUALRESULTS]
    for result_type in sorted(actual_results_dict.keys()):
      results_of_this_type = actual_results_dict[result_type]
      if not results_of_this_type:
        continue
      for image_name in sorted(results_of_this_type.keys()):
        (test, config) = self._image_filename_re.match(image_name).groups()
        (hash_type, hash_digest) = results_of_this_type[image_name]
        source_url = gm_json.CreateGmActualUrl(
            test_name=test, hash_type=hash_type, hash_digest=hash_digest,
            gm_actuals_root_url=self._gm_actuals_root_url)
        dest_path = os.path.join(dest_dir, config, test + '.png')
        copy_contents(source_url=source_url, dest_path=dest_path,
                      create_subdirs_if_needed=True)


def create_filepath_url(filepath):
  """ Returns a file:/// URL pointing at the given filepath on local disk.

  For now, this is only used by unittests, but I anticipate it being useful
  in production, as a way for developers to run rebaseline_server over locally
  generated images.

  TODO(epoger): Move this function, and copy_contents(), into a shared
  utility module.  They are generally useful.

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


def main():
  parser = optparse.OptionParser()
  required_params = []
  parser.add_option('--actuals-base-url',
                    action='store', type='string',
                    default=DEFAULT_ACTUALS_BASE_URL,
                    help=('Base URL from which to read files containing JSON '
                          'summaries of actual GM results; defaults to '
                          '"%default". To get a specific revision (useful for '
                          'trybots) replace "svn" with "svn-history/r123".'))
  # TODO(epoger): Rather than telling the user to run "svn ls" to get the list
  # of builders, add a --list-builders option that will print the list.
  required_params.append('builder')
  parser.add_option('--builder',
                    action='store', type='string',
                    help=('REQUIRED: Which builder to download results for. '
                          'To see a list of builders, run "svn ls %s".' %
                          DEFAULT_ACTUALS_BASE_URL))
  required_params.append('dest_dir')
  parser.add_option('--dest-dir',
                    action='store', type='string',
                    help=('REQUIRED: Directory where all images should be '
                          'written. If this directory does not exist yet, it '
                          'will be created.'))
  parser.add_option('--json-filename',
                    action='store', type='string',
                    default=DEFAULT_JSON_FILENAME,
                    help=('JSON summary filename to read for each builder; '
                          'defaults to "%default".'))
  (params, remaining_args) = parser.parse_args()

  # Make sure all required options were set,
  # and that there were no items left over in the command line.
  for required_param in required_params:
    if not getattr(params, required_param):
      raise Exception('required option \'%s\' was not set' % required_param)
  if len(remaining_args) is not 0:
    raise Exception('extra items specified in the command line: %s' %
                    remaining_args)

  downloader = Download(actuals_base_url=params.actuals_base_url)
  downloader.fetch(builder_name=params.builder,
                   dest_dir=params.dest_dir)



if __name__ == '__main__':
  main()
