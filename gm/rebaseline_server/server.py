#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

HTTP server for our HTML rebaseline viewer.
"""

# System-level imports
import argparse
import BaseHTTPServer
import json
import logging
import os
import posixpath
import re
import shutil
import socket
import subprocess
import thread
import threading
import time
import urlparse

# Imports from within Skia
import fix_pythonpath  # must do this first
from pyutils import gs_utils
import gm_json

# Imports from local dir
#
# Note: we import results under a different name, to avoid confusion with the
# Server.results() property. See discussion at
# https://codereview.chromium.org/195943004/diff/1/gm/rebaseline_server/server.py#newcode44
import compare_configs
import compare_to_expectations
import download_actuals
import imagepairset
import results as results_mod

PATHSPLIT_RE = re.compile('/([^/]+)/(.+)')

# A simple dictionary of file name extensions to MIME types. The empty string
# entry is used as the default when no extension was given or if the extension
# has no entry in this dictionary.
MIME_TYPE_MAP = {'': 'application/octet-stream',
                 'html': 'text/html',
                 'css': 'text/css',
                 'png': 'image/png',
                 'js': 'application/javascript',
                 'json': 'application/json'
                 }

# Keys that server.py uses to create the toplevel content header.
# NOTE: Keep these in sync with static/constants.js
KEY__EDITS__MODIFICATIONS = 'modifications'
KEY__EDITS__OLD_RESULTS_HASH = 'oldResultsHash'
KEY__EDITS__OLD_RESULTS_TYPE = 'oldResultsType'

DEFAULT_ACTUALS_DIR = results_mod.DEFAULT_ACTUALS_DIR
DEFAULT_GM_SUMMARIES_BUCKET = download_actuals.GM_SUMMARIES_BUCKET
DEFAULT_JSON_FILENAME = download_actuals.DEFAULT_JSON_FILENAME
DEFAULT_PORT = 8888

PARENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
TRUNK_DIRECTORY = os.path.dirname(os.path.dirname(PARENT_DIRECTORY))
# Directory, relative to PARENT_DIRECTORY, within which the server will serve
# out live results (not static files).
RESULTS_SUBDIR = 'results'
# Directory, relative to PARENT_DIRECTORY, within which the server will serve
# out static files.
STATIC_CONTENTS_SUBDIR = 'static'
# All of the GENERATED_*_SUBDIRS are relative to STATIC_CONTENTS_SUBDIR
GENERATED_HTML_SUBDIR = 'generated-html'
GENERATED_IMAGES_SUBDIR = 'generated-images'
GENERATED_JSON_SUBDIR = 'generated-json'

# How often (in seconds) clients should reload while waiting for initial
# results to load.
RELOAD_INTERVAL_UNTIL_READY = 10

SUMMARY_TYPES = [
    results_mod.KEY__HEADER__RESULTS_FAILURES,
    results_mod.KEY__HEADER__RESULTS_ALL,
]
# If --compare-configs is specified, compare these configs.
CONFIG_PAIRS_TO_COMPARE = [('8888', 'gpu')]

_HTTP_HEADER_CONTENT_LENGTH = 'Content-Length'
_HTTP_HEADER_CONTENT_TYPE = 'Content-Type'

_SERVER = None   # This gets filled in by main()


def _run_command(args, directory):
  """Runs a command and returns stdout as a single string.

  Args:
    args: the command to run, as a list of arguments
    directory: directory within which to run the command

  Returns: stdout, as a string

  Raises an Exception if the command failed (exited with nonzero return code).
  """
  logging.debug('_run_command: %s in directory %s' % (args, directory))
  proc = subprocess.Popen(args, cwd=directory,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)
  (stdout, stderr) = proc.communicate()
  if proc.returncode is not 0:
    raise Exception('command "%s" failed in dir "%s": %s' %
                    (args, directory, stderr))
  return stdout


def _get_routable_ip_address():
  """Returns routable IP address of this host (the IP address of its network
     interface that would be used for most traffic, not its localhost
     interface).  See http://stackoverflow.com/a/166589 """
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  sock.connect(('8.8.8.8', 80))
  host = sock.getsockname()[0]
  sock.close()
  return host


def _create_index(file_path, config_pairs):
  """Creates an index file linking to all results available from this server.

  Prior to https://codereview.chromium.org/215503002 , we had a static
  index.html within our repo.  But now that the results may or may not include
  config comparisons, index.html needs to be generated differently depending
  on which results are included.

  TODO(epoger): Instead of including raw HTML within the Python code,
  consider restoring the index.html file as a template and using django (or
  similar) to fill in dynamic content.

  Args:
    file_path: path on local disk to write index to; any directory components
               of this path that do not already exist will be created
    config_pairs: what pairs of configs (if any) we compare actual results of
  """
  dir_path = os.path.dirname(file_path)
  if not os.path.isdir(dir_path):
    os.makedirs(dir_path)
  with open(file_path, 'w') as file_handle:
    file_handle.write(
        '<!DOCTYPE html><html>'
        '<head><title>rebaseline_server</title></head>'
        '<body><ul>')
    if SUMMARY_TYPES:
      file_handle.write('<li>Expectations vs Actuals</li><ul>')
      for summary_type in SUMMARY_TYPES:
        file_handle.write(
            '<li>'
            '<a href="/%s/view.html#/view.html?resultsToLoad=/%s/%s">'
            '%s</a></li>' % (
                STATIC_CONTENTS_SUBDIR, RESULTS_SUBDIR,
                summary_type, summary_type))
      file_handle.write('</ul>')
    if config_pairs:
      file_handle.write('<li>Comparing configs within actual results</li><ul>')
      for config_pair in config_pairs:
        file_handle.write('<li>%s vs %s:' % config_pair)
        for summary_type in SUMMARY_TYPES:
          file_handle.write(
              ' <a href="/%s/view.html#/view.html?'
              'resultsToLoad=/%s/%s/%s-vs-%s_%s.json">%s</a>' % (
                  STATIC_CONTENTS_SUBDIR, STATIC_CONTENTS_SUBDIR,
                  GENERATED_JSON_SUBDIR, config_pair[0], config_pair[1],
                  summary_type, summary_type))
        file_handle.write('</li>')
      file_handle.write('</ul>')
    file_handle.write('</ul></body></html>')


class Server(object):
  """ HTTP server for our HTML rebaseline viewer. """

  def __init__(self,
               actuals_dir=DEFAULT_ACTUALS_DIR,
               json_filename=DEFAULT_JSON_FILENAME,
               gm_summaries_bucket=DEFAULT_GM_SUMMARIES_BUCKET,
               port=DEFAULT_PORT, export=False, editable=True,
               reload_seconds=0, config_pairs=None, builder_regex_list=None):
    """
    Args:
      actuals_dir: directory under which we will check out the latest actual
          GM results
      json_filename: basename of the JSON summary file to load for each builder
      gm_summaries_bucket: Google Storage bucket to download json_filename
          files from; if None or '', don't fetch new actual-results files
          at all, just compare to whatever files are already in actuals_dir
      port: which TCP port to listen on for HTTP requests
      export: whether to allow HTTP clients on other hosts to access this server
      editable: whether HTTP clients are allowed to submit new baselines
      reload_seconds: polling interval with which to check for new results;
          if 0, don't check for new results at all
      config_pairs: List of (string, string) tuples; for each tuple, compare
          actual results of these two configs.  If None or empty,
          don't compare configs at all.
      builder_regex_list: List of regular expressions specifying which builders
          we will process. If None, process all builders.
    """
    self._actuals_dir = actuals_dir
    self._json_filename = json_filename
    self._gm_summaries_bucket = gm_summaries_bucket
    self._port = port
    self._export = export
    self._editable = editable
    self._reload_seconds = reload_seconds
    self._config_pairs = config_pairs or []
    self._builder_regex_list = builder_regex_list
    _create_index(
        file_path=os.path.join(
            PARENT_DIRECTORY, STATIC_CONTENTS_SUBDIR, GENERATED_HTML_SUBDIR,
            "index.html"),
        config_pairs=config_pairs)

    # Reentrant lock that must be held whenever updating EITHER of:
    # 1. self._results
    # 2. the expected or actual results on local disk
    self.results_rlock = threading.RLock()
    # self._results will be filled in by calls to update_results()
    self._results = None

  @property
  def results(self):
    """ Returns the most recently generated results, or None if we don't have
    any valid results (update_results() has not completed yet). """
    return self._results

  @property
  def is_exported(self):
    """ Returns true iff HTTP clients on other hosts are allowed to access
    this server. """
    return self._export

  @property
  def is_editable(self):
    """ Returns true iff HTTP clients are allowed to submit new baselines. """
    return self._editable

  @property
  def reload_seconds(self):
    """ Returns the result reload period in seconds, or 0 if we don't reload
    results. """
    return self._reload_seconds

  def update_results(self, invalidate=False):
    """ Create or update self._results, based on the latest expectations and
    actuals.

    We hold self.results_rlock while we do this, to guarantee that no other
    thread attempts to update either self._results or the underlying files at
    the same time.

    Args:
      invalidate: if True, invalidate self._results immediately upon entry;
                  otherwise, we will let readers see those results until we
                  replace them
    """
    with self.results_rlock:
      if invalidate:
        self._results = None
      if self._gm_summaries_bucket:
        logging.info(
            'Updating GM result summaries in %s from gm_summaries_bucket %s ...'
            % (self._actuals_dir, self._gm_summaries_bucket))

        # Clean out actuals_dir first, in case some builders have gone away
        # since we last ran.
        if os.path.isdir(self._actuals_dir):
          shutil.rmtree(self._actuals_dir)

        # Get the list of builders we care about.
        all_builders = download_actuals.get_builders_list(
            summaries_bucket=self._gm_summaries_bucket)
        if self._builder_regex_list:
          matching_builders = []
          for builder in all_builders:
            for regex in self._builder_regex_list:
              if re.match(regex, builder):
                matching_builders.append(builder)
                break  # go on to the next builder, no need to try more regexes
        else:
          matching_builders = all_builders

        # Download the JSON file for each builder we care about.
        #
        # TODO(epoger): When this is a large number of builders, we would be
        # better off downloading them in parallel!
        for builder in matching_builders:
          gs_utils.download_file(
              source_bucket=self._gm_summaries_bucket,
              source_path=posixpath.join(builder, self._json_filename),
              dest_path=os.path.join(self._actuals_dir, builder,
                                     self._json_filename),
              create_subdirs_if_needed=True)

      # We only update the expectations dir if the server was run with a
      # nonzero --reload argument; otherwise, we expect the user to maintain
      # her own expectations as she sees fit.
      #
      # Because the Skia repo is hosted using git, and git does not
      # support updating a single directory tree, we have to update the entire
      # repo checkout.
      #
      # Because Skia uses depot_tools, we have to update using "gclient sync"
      # instead of raw git commands.
      #
      # TODO(epoger): Fetch latest expectations in some other way.
      # Eric points out that our official documentation recommends an
      # unmanaged Skia checkout, so "gclient sync" will not bring down updated
      # expectations from origin/master-- you'd have to do a "git pull" of
      # some sort instead.
      # However, the live rebaseline_server at
      # http://skia-tree-status.appspot.com/redirect/rebaseline-server (which
      # is probably the only user of the --reload flag!) uses a managed
      # checkout, so "gclient sync" works in that case.
      # Probably the best idea is to avoid all of this nonsense by fetching
      # updated expectations into a temp directory, and leaving the rest of
      # the checkout alone.  This could be done using "git show", or by
      # downloading individual expectation JSON files from
      # skia.googlesource.com .
      if self._reload_seconds:
        logging.info(
            'Updating expected GM results in %s by syncing Skia repo ...' %
            compare_to_expectations.DEFAULT_EXPECTATIONS_DIR)
        _run_command(['gclient', 'sync'], TRUNK_DIRECTORY)

      self._results = compare_to_expectations.ExpectationComparisons(
          actuals_root=self._actuals_dir,
          generated_images_root=os.path.join(
              PARENT_DIRECTORY, STATIC_CONTENTS_SUBDIR,
              GENERATED_IMAGES_SUBDIR),
          diff_base_url=posixpath.join(
              os.pardir, STATIC_CONTENTS_SUBDIR, GENERATED_IMAGES_SUBDIR),
          builder_regex_list=self._builder_regex_list)

      json_dir = os.path.join(
          PARENT_DIRECTORY, STATIC_CONTENTS_SUBDIR, GENERATED_JSON_SUBDIR)
      if not os.path.isdir(json_dir):
         os.makedirs(json_dir)

      for config_pair in self._config_pairs:
        config_comparisons = compare_configs.ConfigComparisons(
            configs=config_pair,
            actuals_root=self._actuals_dir,
            generated_images_root=os.path.join(
                PARENT_DIRECTORY, STATIC_CONTENTS_SUBDIR,
                GENERATED_IMAGES_SUBDIR),
            diff_base_url=posixpath.join(
                os.pardir, GENERATED_IMAGES_SUBDIR),
            builder_regex_list=self._builder_regex_list)
        for summary_type in SUMMARY_TYPES:
          gm_json.WriteToFile(
              config_comparisons.get_packaged_results_of_type(
                  results_type=summary_type),
              os.path.join(
                  json_dir, '%s-vs-%s_%s.json' % (
                      config_pair[0], config_pair[1], summary_type)))

  def _result_loader(self, reload_seconds=0):
    """ Call self.update_results(), either once or periodically.

    Params:
      reload_seconds: integer; if nonzero, reload results at this interval
          (in which case, this method will never return!)
    """
    self.update_results()
    logging.info('Initial results loaded. Ready for requests on %s' % self._url)
    if reload_seconds:
      while True:
        time.sleep(reload_seconds)
        self.update_results()

  def run(self):
    arg_tuple = (self._reload_seconds,)  # start_new_thread needs a tuple,
                                         # even though it holds just one param
    thread.start_new_thread(self._result_loader, arg_tuple)

    if self._export:
      server_address = ('', self._port)
      host = _get_routable_ip_address()
      if self._editable:
        logging.warning('Running with combination of "export" and "editable" '
                        'flags.  Users on other machines will '
                        'be able to modify your GM expectations!')
    else:
      host = '127.0.0.1'
      server_address = (host, self._port)
    http_server = BaseHTTPServer.HTTPServer(server_address, HTTPRequestHandler)
    self._url = 'http://%s:%d' % (host, http_server.server_port)
    logging.info('Listening for requests on %s' % self._url)
    http_server.serve_forever()


class HTTPRequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
  """ HTTP request handlers for various types of queries this server knows
      how to handle (static HTML and Javascript, expected/actual results, etc.)
  """
  def do_GET(self):
    """
    Handles all GET requests, forwarding them to the appropriate
    do_GET_* dispatcher.

    If we see any Exceptions, return a 404.  This fixes http://skbug.com/2147
    """
    try:
      logging.debug('do_GET: path="%s"' % self.path)
      if self.path == '' or self.path == '/' or self.path == '/index.html' :
        self.redirect_to('/%s/%s/index.html' % (
            STATIC_CONTENTS_SUBDIR, GENERATED_HTML_SUBDIR))
        return
      if self.path == '/favicon.ico' :
        self.redirect_to('/%s/favicon.ico' % STATIC_CONTENTS_SUBDIR)
        return

      # All requests must be of this form:
      #   /dispatcher/remainder
      # where 'dispatcher' indicates which do_GET_* dispatcher to run
      # and 'remainder' is the remaining path sent to that dispatcher.
      normpath = posixpath.normpath(self.path)
      (dispatcher_name, remainder) = PATHSPLIT_RE.match(normpath).groups()
      dispatchers = {
          RESULTS_SUBDIR: self.do_GET_results,
          STATIC_CONTENTS_SUBDIR: self.do_GET_static,
      }
      dispatcher = dispatchers[dispatcher_name]
      dispatcher(remainder)
    except:
      self.send_error(404)
      raise

  def do_GET_results(self, results_type):
    """ Handle a GET request for GM results.

    Args:
      results_type: string indicating which set of results to return;
            must be one of the results_mod.RESULTS_* constants
    """
    logging.debug('do_GET_results: sending results of type "%s"' % results_type)
    # Since we must make multiple calls to the ExpectationComparisons object,
    # grab a reference to it in case it is updated to point at a new
    # ExpectationComparisons object within another thread.
    #
    # TODO(epoger): Rather than using a global variable for the handler
    # to refer to the Server object, make Server a subclass of
    # HTTPServer, and then it could be available to the handler via
    # the handler's .server instance variable.
    results_obj = _SERVER.results
    if results_obj:
      response_dict = results_obj.get_packaged_results_of_type(
          results_type=results_type, reload_seconds=_SERVER.reload_seconds,
          is_editable=_SERVER.is_editable, is_exported=_SERVER.is_exported)
    else:
      now = int(time.time())
      response_dict = {
          imagepairset.KEY__ROOT__HEADER: {
              results_mod.KEY__HEADER__SCHEMA_VERSION: (
                  results_mod.VALUE__HEADER__SCHEMA_VERSION),
              results_mod.KEY__HEADER__IS_STILL_LOADING: True,
              results_mod.KEY__HEADER__TIME_UPDATED: now,
              results_mod.KEY__HEADER__TIME_NEXT_UPDATE_AVAILABLE: (
                  now + RELOAD_INTERVAL_UNTIL_READY),
          },
      }
    self.send_json_dict(response_dict)

  def do_GET_static(self, path):
    """ Handle a GET request for a file under STATIC_CONTENTS_SUBDIR .
    Only allow serving of files within STATIC_CONTENTS_SUBDIR that is a
    filesystem sibling of this script.

    Args:
      path: path to file (within STATIC_CONTENTS_SUBDIR) to retrieve
    """
    # Strip arguments ('?resultsToLoad=all') from the path
    path = urlparse.urlparse(path).path

    logging.debug('do_GET_static: sending file "%s"' % path)
    static_dir = os.path.realpath(os.path.join(
        PARENT_DIRECTORY, STATIC_CONTENTS_SUBDIR))
    full_path = os.path.realpath(os.path.join(static_dir, path))
    if full_path.startswith(static_dir):
      self.send_file(full_path)
    else:
      logging.error(
          'Attempted do_GET_static() of path [%s] outside of static dir [%s]'
          % (full_path, static_dir))
      self.send_error(404)

  def do_POST(self):
    """ Handles all POST requests, forwarding them to the appropriate
        do_POST_* dispatcher. """
    # All requests must be of this form:
    #   /dispatcher
    # where 'dispatcher' indicates which do_POST_* dispatcher to run.
    logging.debug('do_POST: path="%s"' % self.path)
    normpath = posixpath.normpath(self.path)
    dispatchers = {
      '/edits': self.do_POST_edits,
    }
    try:
      dispatcher = dispatchers[normpath]
      dispatcher()
      self.send_response(200)
    except:
      self.send_error(404)
      raise

  def do_POST_edits(self):
    """ Handle a POST request with modifications to GM expectations, in this
    format:

    {
      KEY__EDITS__OLD_RESULTS_TYPE: 'all',  # type of results that the client
                                            # loaded and then made
                                            # modifications to
      KEY__EDITS__OLD_RESULTS_HASH: 39850913, # hash of results when the client
                                              # loaded them (ensures that the
                                              # client and server apply
                                              # modifications to the same base)
      KEY__EDITS__MODIFICATIONS: [
        # as needed by compare_to_expectations.edit_expectations()
        ...
      ],
    }

    Raises an Exception if there were any problems.
    """
    if not _SERVER.is_editable:
      raise Exception('this server is not running in --editable mode')

    content_type = self.headers[_HTTP_HEADER_CONTENT_TYPE]
    if content_type != 'application/json;charset=UTF-8':
      raise Exception('unsupported %s [%s]' % (
          _HTTP_HEADER_CONTENT_TYPE, content_type))

    content_length = int(self.headers[_HTTP_HEADER_CONTENT_LENGTH])
    json_data = self.rfile.read(content_length)
    data = json.loads(json_data)
    logging.debug('do_POST_edits: received new GM expectations data [%s]' %
                  data)

    # Update the results on disk with the information we received from the
    # client.
    # We must hold _SERVER.results_rlock while we do this, to guarantee that
    # no other thread updates expectations (from the Skia repo) while we are
    # updating them (using the info we received from the client).
    with _SERVER.results_rlock:
      oldResultsType = data[KEY__EDITS__OLD_RESULTS_TYPE]
      oldResults = _SERVER.results.get_results_of_type(oldResultsType)
      oldResultsHash = str(hash(repr(
          oldResults[imagepairset.KEY__ROOT__IMAGEPAIRS])))
      if oldResultsHash != data[KEY__EDITS__OLD_RESULTS_HASH]:
        raise Exception('results of type "%s" changed while the client was '
                        'making modifications. The client should reload the '
                        'results and submit the modifications again.' %
                        oldResultsType)
      _SERVER.results.edit_expectations(data[KEY__EDITS__MODIFICATIONS])

    # Read the updated results back from disk.
    # We can do this in a separate thread; we should return our success message
    # to the UI as soon as possible.
    thread.start_new_thread(_SERVER.update_results, (True,))

  def redirect_to(self, url):
    """ Redirect the HTTP client to a different url.

    Args:
      url: URL to redirect the HTTP client to
    """
    self.send_response(301)
    self.send_header('Location', url)
    self.end_headers()

  def send_file(self, path):
    """ Send the contents of the file at this path, with a mimetype based
        on the filename extension.

    Args:
      path: path of file whose contents to send to the HTTP client
    """
    # Grab the extension if there is one
    extension = os.path.splitext(path)[1]
    if len(extension) >= 1:
      extension = extension[1:]

    # Determine the MIME type of the file from its extension
    mime_type = MIME_TYPE_MAP.get(extension, MIME_TYPE_MAP[''])

    # Open the file and send it over HTTP
    if os.path.isfile(path):
      with open(path, 'rb') as sending_file:
        self.send_response(200)
        self.send_header('Content-type', mime_type)
        self.end_headers()
        self.wfile.write(sending_file.read())
    else:
      self.send_error(404)

  def send_json_dict(self, json_dict):
    """ Send the contents of this dictionary in JSON format, with a JSON
        mimetype.

    Args:
      json_dict: dictionary to send
    """
    self.send_response(200)
    self.send_header('Content-type', 'application/json')
    self.end_headers()
    json.dump(json_dict, self.wfile)


def main():
  logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                      datefmt='%m/%d/%Y %H:%M:%S',
                      level=logging.INFO)
  parser = argparse.ArgumentParser()
  parser.add_argument('--actuals-dir',
                    help=('Directory into which we will check out the latest '
                          'actual GM results. If this directory does not '
                          'exist, it will be created. Defaults to %(default)s'),
                    default=DEFAULT_ACTUALS_DIR)
  # TODO(epoger): Before https://codereview.chromium.org/310093003 ,
  # when this tool downloaded the JSON summaries from skia-autogen,
  # it had an --actuals-revision the caller could specify to download
  # actual results as of a specific point in time.  We should add similar
  # functionality when retrieving the summaries from Google Storage.
  parser.add_argument('--builders', metavar='BUILDER_REGEX', nargs='+',
                      help=('Only process builders matching these regular '
                            'expressions.  If unspecified, process all '
                            'builders.'))
  parser.add_argument('--compare-configs', action='store_true',
                      help=('In addition to generating differences between '
                            'expectations and actuals, also generate '
                            'differences between these config pairs: '
                            + str(CONFIG_PAIRS_TO_COMPARE)))
  parser.add_argument('--editable', action='store_true',
                      help=('Allow HTTP clients to submit new baselines.'))
  parser.add_argument('--export', action='store_true',
                      help=('Instead of only allowing access from HTTP clients '
                            'on localhost, allow HTTP clients on other hosts '
                            'to access this server.  WARNING: doing so will '
                            'allow users on other hosts to modify your '
                            'GM expectations, if combined with --editable.'))
  parser.add_argument('--gm-summaries-bucket',
                    help=('Google Cloud Storage bucket to download '
                          'JSON_FILENAME files from. '
                          'Defaults to %(default)s ; if set to '
                          'empty string, just compare to actual-results '
                          'already found in ACTUALS_DIR.'),
                    default=DEFAULT_GM_SUMMARIES_BUCKET)
  parser.add_argument('--json-filename',
                    help=('JSON summary filename to read for each builder; '
                          'defaults to %(default)s.'),
                    default=DEFAULT_JSON_FILENAME)
  parser.add_argument('--port', type=int,
                      help=('Which TCP port to listen on for HTTP requests; '
                            'defaults to %(default)s'),
                      default=DEFAULT_PORT)
  parser.add_argument('--reload', type=int,
                      help=('How often (a period in seconds) to update the '
                            'results.  If specified, both expected and actual '
                            'results will be updated by running "gclient sync" '
                            'on your Skia checkout as a whole.  '
                            'By default, we do not reload at all, and you '
                            'must restart the server to pick up new data.'),
                      default=0)
  args = parser.parse_args()
  if args.compare_configs:
    config_pairs = CONFIG_PAIRS_TO_COMPARE
  else:
    config_pairs = None

  global _SERVER
  _SERVER = Server(actuals_dir=args.actuals_dir,
                   json_filename=args.json_filename,
                   gm_summaries_bucket=args.gm_summaries_bucket,
                   port=args.port, export=args.export, editable=args.editable,
                   reload_seconds=args.reload, config_pairs=config_pairs,
                   builder_regex_list=args.builders)
  _SERVER.run()


if __name__ == '__main__':
  main()
