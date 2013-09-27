#!/usr/bin/python

'''
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

'''
HTTP server for our HTML rebaseline viewer.
'''

# System-level imports
import argparse
import BaseHTTPServer
import json
import os
import posixpath
import re
import shutil
import sys

# Imports from within Skia
#
# We need to add the 'tools' directory, so that we can import svn.py within
# that directory.
# Make sure that the 'tools' dir is in the PYTHONPATH, but add it at the *end*
# so any dirs that are already in the PYTHONPATH will be preferred.
TRUNK_DIRECTORY = os.path.dirname(os.path.dirname(os.path.dirname(
    os.path.realpath(__file__))))
TOOLS_DIRECTORY = os.path.join(TRUNK_DIRECTORY, 'tools')
if TOOLS_DIRECTORY not in sys.path:
  sys.path.append(TOOLS_DIRECTORY)
import svn

# Imports from local dir
import results

ACTUALS_SVN_REPO = 'http://skia-autogen.googlecode.com/svn/gm-actual'
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
  """ HTTP server for our HTML rebaseline viewer.

  params:
    actuals_dir: directory under which we will check out the latest actual
                 GM results
    expectations_dir: directory under which to find GM expectations (they
                      must already be in that directory)
    port: which TCP port to listen on for HTTP requests
    export: whether to allow HTTP clients on other hosts to access this server
  """
  def __init__(self,
               actuals_dir=DEFAULT_ACTUALS_DIR,
               expectations_dir=DEFAULT_EXPECTATIONS_DIR,
               port=DEFAULT_PORT, export=False):
    self._actuals_dir = actuals_dir
    self._expectations_dir = expectations_dir
    self._port = port
    self._export = export

  def fetch_results(self):
    """ Create self.results, based on the expectations in
    self._expectations_dir and the latest actuals from skia-autogen.

    TODO(epoger): Add a new --browseonly mode setting.  In that mode,
    the gm-actuals and expectations will automatically be updated every few
    minutes.  See discussion in https://codereview.chromium.org/24274003/ .
    """
    print 'Checking out latest actual GM results from %s into %s ...' % (
        ACTUALS_SVN_REPO, self._actuals_dir)
    actuals_repo = svn.Svn(self._actuals_dir)
    if not os.path.isdir(self._actuals_dir):
      os.makedirs(self._actuals_dir)
      actuals_repo.Checkout(ACTUALS_SVN_REPO, '.')
    else:
      actuals_repo.Update('.')
    print 'Parsing results from actuals in %s and expectations in %s ...' % (
        self._actuals_dir, self._expectations_dir)
    self.results = results.Results(
      actuals_root=self._actuals_dir,
      expected_root=self._expectations_dir)

  def run(self):
    self.fetch_results()
    if self._export:
      server_address = ('', self._port)
      print ('WARNING: Running in "export" mode. Users on other machines will '
             'be able to modify your GM expectations!')
    else:
      server_address = ('127.0.0.1', self._port)
    http_server = BaseHTTPServer.HTTPServer(server_address, HTTPRequestHandler)
    print 'Ready for requests on http://%s:%d' % (
        http_server.server_name, http_server.server_port)
    http_server.serve_forever()


class HTTPRequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
  """ HTTP request handlers for various types of queries this server knows
      how to handle (static HTML and Javascript, expected/actual results, etc.)
  """
  def do_GET(self):
    """ Handles all GET requests, forwarding them to the appropriate
        do_GET_* dispatcher. """
    if self.path == '' or self.path == '/' or self.path == '/index.html' :
      self.redirect_to('/static/view.html')
      return
    if self.path == '/favicon.ico' :
      self.redirect_to('/static/favicon.ico')
      return

    # All requests must be of this form:
    #   /dispatcher/remainder
    # where "dispatcher" indicates which do_GET_* dispatcher to run
    # and "remainder" is the remaining path sent to that dispatcher.
    normpath = posixpath.normpath(self.path)
    (dispatcher_name, remainder) = PATHSPLIT_RE.match(normpath).groups()
    dispatchers = {
      'results': self.do_GET_results,
      'static': self.do_GET_static,
    }
    dispatcher = dispatchers[dispatcher_name]
    dispatcher(remainder)

  def do_GET_results(self, result_type):
    """ Handle a GET request for GM results.
    For now, we ignore the remaining path info, because we only know how to
    return all results.

    TODO(epoger): Unless we start making use of result_type, remove that
    parameter."""
    print 'do_GET_results: sending results of type "%s"' % result_type
    response_dict = _SERVER.results.GetAll()
    if response_dict:
      self.send_json_dict(response_dict)
    else:
      self.send_error(404)

  def do_GET_static(self, path):
    """ Handle a GET request for a file under the 'static' directory. """
    print 'do_GET_static: sending file "%s"' % path
    self.send_file(posixpath.join('static', path))

  def redirect_to(self, url):
    """ Redirect the HTTP client to a different url. """
    self.send_response(301)
    self.send_header('Location', url)
    self.end_headers()

  def send_file(self, path):
    """ Send the contents of the file at this path, with a mimetype based
        on the filename extension. """
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
        mimetype. """
    self.send_response(200)
    self.send_header('Content-type', 'application/json')
    self.end_headers()
    json.dump(json_dict, self.wfile)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--actuals-dir',
                    help=('Directory into which we will check out the latest '
                          'actual GM results. If this directory does not '
                          'exist, it will be created. Defaults to %(default)s'),
                    default=DEFAULT_ACTUALS_DIR)
  parser.add_argument('--expectations-dir',
                    help=('Directory under which to find GM expectations; '
                          'defaults to %(default)s'),
                    default=DEFAULT_EXPECTATIONS_DIR)
  parser.add_argument('--export', action='store_true',
                      help=('Instead of only allowing access from HTTP clients '
                            'on localhost, allow HTTP clients on other hosts '
                            'to access this server.  WARNING: doing so will '
                            'allow users on other hosts to modify your '
                            'GM expectations!'))
  parser.add_argument('--port',
                    help=('Which TCP port to listen on for HTTP requests; '
                          'defaults to %(default)s'),
                    default=DEFAULT_PORT)
  args = parser.parse_args()
  global _SERVER
  _SERVER = Server(expectations_dir=args.expectations_dir,
                   port=args.port, export=args.export)
  _SERVER.run()

if __name__ == '__main__':
  main()
