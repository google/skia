#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""A tiny web server.

This is intended to be used for testing, and only run from within the examples
directory.
"""

import BaseHTTPServer
import logging
import optparse
import os
import SimpleHTTPServer
import SocketServer
import sys
import urlparse


EXAMPLE_PATH=os.path.dirname(os.path.abspath(__file__))
NACL_SDK_ROOT = os.getenv('NACL_SDK_ROOT', os.path.dirname(EXAMPLE_PATH))


if os.path.exists(NACL_SDK_ROOT):
  sys.path.append(os.path.join(NACL_SDK_ROOT, 'tools'))
  import decode_dump
  import getos
else:
  NACL_SDK_ROOT=None

last_nexe = None
last_nmf = None

logging.getLogger().setLevel(logging.INFO)

# Using 'localhost' means that we only accept connections
# via the loop back interface.
SERVER_PORT = 5103
SERVER_HOST = ''

# We only run from the examples directory so that not too much is exposed
# via this HTTP server.  Everything in the directory is served, so there should
# never be anything potentially sensitive in the serving directory, especially
# if the machine might be a multi-user machine and not all users are trusted.
# We only serve via the loopback interface.
def SanityCheckDirectory():
  httpd_path = os.path.abspath(os.path.dirname(__file__))
  serve_path = os.path.abspath(os.getcwd())

  # Verify we are serving from the directory this script came from, or bellow
  if serve_path[:len(httpd_path)] == httpd_path:
    return
  logging.error('For security, httpd.py should only be run from within the')
  logging.error('example directory tree.')
  logging.error('We are currently in %s.' % serve_path)
  sys.exit(1)


# An HTTP server that will quit when |is_running| is set to False.  We also use
# SocketServer.ThreadingMixIn in order to handle requests asynchronously for
# faster responses.
class QuittableHTTPServer(SocketServer.ThreadingMixIn,
                          BaseHTTPServer.HTTPServer):
  def serve_forever(self, timeout=0.5):
    self.is_running = True
    self.timeout = timeout
    while self.is_running:
      self.handle_request()

  def shutdown(self):
    self.is_running = False
    return 1


# "Safely" split a string at |sep| into a [key, value] pair.  If |sep| does not
# exist in |str|, then the entire |str| is the key and the value is set to an
# empty string.
def KeyValuePair(str, sep='='):
  if sep in str:
    return str.split(sep)
  else:
    return [str, '']


# A small handler that looks for '?quit=1' query in the path and shuts itself
# down if it finds that parameter.
class QuittableHTTPHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
  def send_head(self):
    """Common code for GET and HEAD commands.

    This sends the response code and MIME headers.

    Return value is either a file object (which has to be copied
    to the outputfile by the caller unless the command was HEAD,
    and must be closed by the caller under all circumstances), or
    None, in which case the caller has nothing further to do.

    """
    path = self.translate_path(self.path)
    f = None
    if os.path.isdir(path):
        if not self.path.endswith('/'):
            # redirect browser - doing basically what apache does
            self.send_response(301)
            self.send_header("Location", self.path + "/")
            self.end_headers()
            return None
        for index in "index.html", "index.htm":
            index = os.path.join(path, index)
            if os.path.exists(index):
                path = index
                break
        else:
            return self.list_directory(path)
    ctype = self.guess_type(path)
    try:
        # Always read in binary mode. Opening files in text mode may cause
        # newline translations, making the actual size of the content
        # transmitted *less* than the content-length!
        f = open(path, 'rb')
    except IOError:
        self.send_error(404, "File not found")
        return None
    self.send_response(200)
    self.send_header("Content-type", ctype)
    fs = os.fstat(f.fileno())
    self.send_header("Content-Length", str(fs[6]))
    self.send_header("Last-Modified", self.date_time_string(fs.st_mtime))
    self.send_header('Cache-Control','no-cache, must-revalidate')
    self.send_header('Expires','-1')
    self.end_headers()
    return f

  def do_GET(self):
    global last_nexe, last_nmf
    (_, _, path, query, _) = urlparse.urlsplit(self.path)
    url_params = dict([KeyValuePair(key_value)
                      for key_value in query.split('&')])
    if 'quit' in url_params and '1' in url_params['quit']:
      self.send_response(200, 'OK')
      self.send_header('Content-type', 'text/html')
      self.send_header('Content-length', '0')
      self.end_headers()
      self.server.shutdown()
      return

    if path.endswith('.nexe'):
      last_nexe = path
    if path.endswith('.nmf'):
      last_nmf = path

    SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

  def do_POST(self):
    (_, _,path, query, _) = urlparse.urlsplit(self.path)
    if 'Content-Length' in self.headers:
      if not NACL_SDK_ROOT:
        self.wfile('Could not find NACL_SDK_ROOT to decode trace.')
        return
      data = self.rfile.read(int(self.headers['Content-Length']))
      nexe = '.' + last_nexe
      nmf = '.' + last_nmf
      addr = os.path.join(NACL_SDK_ROOT, 'toolchain', 
                          getos.GetPlatform() + '_x86_newlib', 
                          'bin', 'x86_64-nacl-addr2line')
      decoder = decode_dump.CoreDecoder(nexe, nmf, addr, None, None)
      info = decoder.Decode(data)
      trace = decoder.StackTrace(info)
      decoder.PrintTrace(trace, sys.stdout)
      decoder.PrintTrace(trace, self.wfile)


def Run(server_address,
        server_class=QuittableHTTPServer,
        handler_class=QuittableHTTPHandler):
  httpd = server_class(server_address, handler_class)
  logging.info("Starting local server on port %d", server_address[1])
  logging.info("To shut down send http://localhost:%d?quit=1",
               server_address[1])
  try:
    httpd.serve_forever()
  except KeyboardInterrupt:
    logging.info("Received keyboard interrupt.")
    httpd.server_close()

  logging.info("Shutting down local server on port %d", server_address[1])


def main():
  usage_str = "usage: %prog [options] [optional_portnum]"
  parser = optparse.OptionParser(usage=usage_str)
  parser.add_option(
    '--no_dir_check', dest='do_safe_check',
    action='store_false', default=True,
    help='Do not ensure that httpd.py is being run from a safe directory.')
  (options, args) = parser.parse_args(sys.argv)
  if options.do_safe_check:
    SanityCheckDirectory()
  if len(args) > 2:
    print 'Too many arguments specified.'
    parser.print_help()
  elif len(args) == 2:
    Run((SERVER_HOST, int(args[1])))
  else:
    Run((SERVER_HOST, SERVER_PORT))
  return 0


if __name__ == '__main__':
  sys.exit(main())
