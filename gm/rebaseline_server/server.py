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
import sys
import thread
import time
import urlparse

# Imports from within Skia
#
# We need to add the 'tools' directory, so that we can import svn.py within
# that directory.
# Make sure that the 'tools' dir is in the PYTHONPATH, but add it at the *end*
# so any dirs that are already in the PYTHONPATH will be preferred.
PARENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
TRUNK_DIRECTORY = os.path.dirname(os.path.dirname(PARENT_DIRECTORY))
TOOLS_DIRECTORY = os.path.join(TRUNK_DIRECTORY, 'tools')
if TOOLS_DIRECTORY not in sys.path:
  sys.path.append(TOOLS_DIRECTORY)
import svn

# Imports from local dir
import results

ACTUALS_SVN_REPO = 'http://skia-autogen.googlecode.com/svn/gm-actual'
EXPECTATIONS_SVN_REPO = 'http://skia.googlecode.com/svn/trunk/expectations/gm'
PATHSPLIT_RE = re.compile('/([^/]+)/(.+)')
TRUNK_DIRECTORY = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.realpath(__file__))))

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

DEFAULT_ACTUALS_DIR = '.gm-actuals'
DEFAULT_EXPECTATIONS_DIR = os.path.join(TRUNK_DIRECTORY, 'expectations', 'gm')
DEFAULT_PORT = 8888

_SERVER = None   # This gets filled in by main()

class Server(object):
  """ HTTP server for our HTML rebaseline viewer. """

  def __init__(self,
               actuals_dir=DEFAULT_ACTUALS_DIR,
               expectations_dir=DEFAULT_EXPECTATIONS_DIR,
               port=DEFAULT_PORT, export=False, editable=True,
               reload_seconds=0):
    """
    Args:
      actuals_dir: directory under which we will check out the latest actual
                   GM results
      expectations_dir: directory under which to find GM expectations (they
                        must already be in that directory)
      port: which TCP port to listen on for HTTP requests
      export: whether to allow HTTP clients on other hosts to access this server
      editable: whether HTTP clients are allowed to submit new baselines
      reload_seconds: polling interval with which to check for new results;
                      if 0, don't check for new results at all
    """
    self._actuals_dir = actuals_dir
    self._expectations_dir = expectations_dir
    self._port = port
    self._export = export
    self._editable = editable
    self._reload_seconds = reload_seconds

  def is_exported(self):
    """ Returns true iff HTTP clients on other hosts are allowed to access
    this server. """
    return self._export

  def is_editable(self):
    """ Returns true iff HTTP clients are allowed to submit new baselines. """
    return self._editable

  def reload_seconds(self):
    """ Returns the result reload period in seconds, or 0 if we don't reload
    results. """
    return self._reload_seconds

  def _update_results(self):
    """ Create or update self.results, based on the expectations in
    self._expectations_dir and the latest actuals from skia-autogen.
    """
    logging.info('Updating actual GM results in %s from SVN repo %s ...' % (
        self._actuals_dir, ACTUALS_SVN_REPO))
    actuals_repo = svn.Svn(self._actuals_dir)
    if not os.path.isdir(self._actuals_dir):
      os.makedirs(self._actuals_dir)
      actuals_repo.Checkout(ACTUALS_SVN_REPO, '.')
    else:
      actuals_repo.Update('.')

    # We only update the expectations dir if the server was run with a nonzero
    # --reload argument; otherwise, we expect the user to maintain her own
    # expectations as she sees fit.
    #
    # TODO(epoger): Use git instead of svn to check out expectations, since
    # the Skia repo is moving to git.
    if self._reload_seconds:
      logging.info('Updating expected GM results in %s from SVN repo %s ...' % (
          self._expectations_dir, EXPECTATIONS_SVN_REPO))
      expectations_repo = svn.Svn(self._expectations_dir)
      if not os.path.isdir(self._expectations_dir):
        os.makedirs(self._expectations_dir)
        expectations_repo.Checkout(EXPECTATIONS_SVN_REPO, '.')
      else:
        expectations_repo.Update('.')

    logging.info(
        'Parsing results from actuals in %s and expectations in %s ...' % (
        self._actuals_dir, self._expectations_dir))
    self.results = results.Results(
      actuals_root=self._actuals_dir,
      expected_root=self._expectations_dir)

  def _result_reloader(self):
    """ If --reload argument was specified, reload results at the appropriate
    interval.
    """
    while self._reload_seconds:
      time.sleep(self._reload_seconds)
      with self.results_lock:
        self._update_results()

  def run(self):
    self._update_results()
    self.results_lock = thread.allocate_lock()
    thread.start_new_thread(self._result_reloader, ())

    if self._export:
      server_address = ('', self._port)
      if self._editable:
        logging.warning('Running with combination of "export" and "editable" '
                        'flags.  Users on other machines will '
                        'be able to modify your GM expectations!')
    else:
      server_address = ('127.0.0.1', self._port)
    http_server = BaseHTTPServer.HTTPServer(server_address, HTTPRequestHandler)
    logging.info('Ready for requests on http://%s:%d' % (
        http_server.server_name, http_server.server_port))
    http_server.serve_forever()


class HTTPRequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
  """ HTTP request handlers for various types of queries this server knows
      how to handle (static HTML and Javascript, expected/actual results, etc.)
  """
  def do_GET(self):
    """ Handles all GET requests, forwarding them to the appropriate
        do_GET_* dispatcher. """
    if self.path == '' or self.path == '/' or self.path == '/index.html' :
      self.redirect_to('/static/view.html?resultsToLoad=all')
      return
    if self.path == '/favicon.ico' :
      self.redirect_to('/static/favicon.ico')
      return

    # All requests must be of this form:
    #   /dispatcher/remainder
    # where 'dispatcher' indicates which do_GET_* dispatcher to run
    # and 'remainder' is the remaining path sent to that dispatcher.
    normpath = posixpath.normpath(self.path)
    (dispatcher_name, remainder) = PATHSPLIT_RE.match(normpath).groups()
    dispatchers = {
      'results': self.do_GET_results,
      'static': self.do_GET_static,
    }
    dispatcher = dispatchers[dispatcher_name]
    dispatcher(remainder)

  def do_GET_results(self, type):
    """ Handle a GET request for GM results.

    Args:
      type: string indicating which set of results to return;
            must be one of the results.RESULTS_* constants
    """
    logging.debug('do_GET_results: sending results of type "%s"' % type)
    try:
      # TODO(epoger): Rather than using a global variable for the handler
      # to refer to the Server object, make Server a subclass of
      # HTTPServer, and then it could be available to the handler via
      # the handler's .server instance variable.

      with _SERVER.results_lock:
        response_dict = _SERVER.results.get_results_of_type(type)
        time_updated = _SERVER.results.get_timestamp()
      response_dict['header'] = {
        # Timestamps:
        # 1. when this data was last updated
        # 2. when the caller should check back for new data (if ever)
        #
        # We only return these timestamps if the --reload argument was passed;
        # otherwise, we have no idea when the expectations were last updated
        # (we allow the user to maintain her own expectations as she sees fit).
        'timeUpdated': time_updated if _SERVER.reload_seconds() else None,
        'timeNextUpdateAvailable': (
            (time_updated+_SERVER.reload_seconds()) if _SERVER.reload_seconds()
            else None),

        # Hash of testData, which the client must return with any edits--
        # this ensures that the edits were made to a particular dataset.
        'dataHash': str(hash(repr(response_dict['testData']))),

        # Whether the server will accept edits back.
        'isEditable': _SERVER.is_editable(),

        # Whether the service is accessible from other hosts.
        'isExported': _SERVER.is_exported(),
      }
      self.send_json_dict(response_dict)
    except:
      self.send_error(404)
      raise

  def do_GET_static(self, path):
    """ Handle a GET request for a file under the 'static' directory.
    Only allow serving of files within the 'static' directory that is a
    filesystem sibling of this script.

    Args:
      path: path to file (under static directory) to retrieve
    """
    # Strip arguments ('?resultsToLoad=all') from the path
    path = urlparse.urlparse(path).path

    logging.debug('do_GET_static: sending file "%s"' % path)
    static_dir = os.path.realpath(os.path.join(PARENT_DIRECTORY, 'static'))
    full_path = os.path.realpath(os.path.join(static_dir, path))
    if full_path.startswith(static_dir):
      self.send_file(full_path)
    else:
      logging.error(
          'Attempted do_GET_static() of path [%s] outside of static dir [%s]'
          % (full_path, static_dir))
      self.send_error(404)

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
  logging.basicConfig(level=logging.INFO)
  parser = argparse.ArgumentParser()
  parser.add_argument('--actuals-dir',
                    help=('Directory into which we will check out the latest '
                          'actual GM results. If this directory does not '
                          'exist, it will be created. Defaults to %(default)s'),
                    default=DEFAULT_ACTUALS_DIR)
  parser.add_argument('--editable', action='store_true',
                      help=('TODO(epoger): NOT YET IMPLEMENTED.  '
                            'Allow HTTP clients to submit new baselines.'))
  parser.add_argument('--expectations-dir',
                    help=('Directory under which to find GM expectations; '
                          'defaults to %(default)s'),
                    default=DEFAULT_EXPECTATIONS_DIR)
  parser.add_argument('--export', action='store_true',
                      help=('Instead of only allowing access from HTTP clients '
                            'on localhost, allow HTTP clients on other hosts '
                            'to access this server.  WARNING: doing so will '
                            'allow users on other hosts to modify your '
                            'GM expectations, if combined with --editable.'))
  parser.add_argument('--port', type=int,
                      help=('Which TCP port to listen on for HTTP requests; '
                            'defaults to %(default)s'),
                      default=DEFAULT_PORT)
  parser.add_argument('--reload', type=int,
                      help=('How often (a period in seconds) to update the '
                            'results.  If specified, both EXPECTATIONS_DIR and '
                            'ACTUAL_DIR will be updated.  '
                            'By default, we do not reload at all, and you '
                            'must restart the server to pick up new data.'),
                      default=0)
  args = parser.parse_args()
  global _SERVER
  _SERVER = Server(actuals_dir=args.actuals_dir,
                   expectations_dir=args.expectations_dir,
                   port=args.port, export=args.export, editable=args.editable,
                   reload_seconds=args.reload)
  _SERVER.run()

if __name__ == '__main__':
  main()
