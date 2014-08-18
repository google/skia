#!/usr/bin/python

"""
Copyright 2014 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

Download actual GM results for a particular builder.
"""

# System-level imports
import optparse
import os
import posixpath
import re
import urllib2

# Must fix up PYTHONPATH before importing from within Skia
import rs_fixpypath  # pylint: disable=W0611

# Imports from within Skia
from py.utils import gs_utils
from py.utils import url_utils
import buildbot_globals
import gm_json


GM_SUMMARIES_BUCKET = buildbot_globals.Get('gm_summaries_bucket')
DEFAULT_ACTUALS_BASE_URL = (
    'http://storage.googleapis.com/%s' % GM_SUMMARIES_BUCKET)
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
        url_utils.copy_contents(source_url=source_url, dest_path=dest_path,
                                create_subdirs_if_needed=True)


def get_builders_list(summaries_bucket=GM_SUMMARIES_BUCKET):
  """ Returns the list of builders we have actual results for.

  Args:
    summaries_bucket: Google Cloud Storage bucket containing the summary
        JSON files
  """
  dirs, _ = gs_utils.GSUtils().list_bucket_contents(bucket=GM_SUMMARIES_BUCKET)
  return dirs


def main():
  parser = optparse.OptionParser()
  required_params = []
  parser.add_option('--actuals-base-url',
                    action='store', type='string',
                    default=DEFAULT_ACTUALS_BASE_URL,
                    help=('Base URL from which to read files containing JSON '
                          'summaries of actual GM results; defaults to '
                          '"%default".'))
  required_params.append('builder')
  # TODO(epoger): Before https://codereview.chromium.org/309653005 , when this
  # tool downloaded the JSON summaries from skia-autogen, it had the ability
  # to get results as of a specific revision number.  We should add similar
  # functionality when retrieving the summaries from Google Storage.
  parser.add_option('--builder',
                    action='store', type='string',
                    help=('REQUIRED: Which builder to download results for. '
                          'To see a list of builders, run with the '
                          '--list-builders option set.'))
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
  parser.add_option('--list-builders', action='store_true',
                    help=('List all available builders.'))
  (params, remaining_args) = parser.parse_args()

  if params.list_builders:
    print '\n'.join(get_builders_list())
    return

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
