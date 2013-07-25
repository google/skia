#!/usr/bin/python
# -*- coding: utf-8 -*-

from __future__ import print_function
import BaseHTTPServer
import os
import os.path

# A simple dictionary of file name extensions to MIME types. The empty string
# entry is used as the default when no extension was given or if the extension
# has no entry in this dictionary.
MIME_TYPE_MAP = {'': 'application/octet-stream',
                 'html': 'text/html',
                 'css': 'text/css',
                 'png': 'image/png',
                 'js': 'application/javascript'
                 }


class SkPDiffHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def send_file(self, file_path):
        # Grab the extension if there is one
        extension = os.path.splitext(file_path)[1]
        if len(extension) >= 1:
            extension = extension[1:]

        # Determine the MIME type of the file from its extension
        mime_type = MIME_TYPE_MAP.get(extension, MIME_TYPE_MAP[''])

        # Open the file and send it over HTTP
        sending_file = open(file_path, 'rb')
        self.send_response(200)
        self.send_header('Content-type', mime_type)
        self.end_headers()
        self.wfile.write(sending_file.read())
        sending_file.close()

    def serve_if_in_dir(self, dir_path, file_path):
        # Determine if the file exists relative to the given dir_path AND exists
        # under the dir_path. This is to prevent accidentally serving files
        # outside the directory intended using symlinks, or '../'.
        real_path = os.path.normpath(os.path.join(dir_path, file_path))
        print(repr(real_path))
        if os.path.commonprefix([real_path, dir_path]) == dir_path:
            if os.path.isfile(real_path):
                self.send_file(real_path)
                return True
        return False

    def do_GET(self):
        # Grab the script path because that is where all the static assets are
        script_dir = os.path.dirname(os.path.abspath(__file__))

        # Simple rewrite rule of the root path to 'viewer.html'
        if self.path == '' or self.path == '/':
            self.path = '/viewer.html'

        # The [1:] chops off the leading '/'
        file_path = self.path[1:]

        # Attempt to send static asset files first
        if self.serve_if_in_dir(script_dir, file_path):
            return

        # WARNING: Using the root is a big ol' hack. Incredibly insecure. Only
        # allow serving to localhost unless you want the network to be able to
        # see ALL of the files you can.
        # Attempt to send gm image files
        if self.serve_if_in_dir('/', file_path):
            return

        # If no file to send was found, just give the standard 404
        self.send_error(404)


def main():
    # Do not bind to interfaces other than localhost because the server will
    # attempt to serve files relative to the root directory as a last resort
    # before 404ing. This means all of your files can be accessed from this
    # server, so DO NOT let this server listen to anything but localhost.
    server_address = ('127.0.0.1', 8080)
    http_server = BaseHTTPServer.HTTPServer(server_address, SkPDiffHandler)
    print('Navigate thine browser to: {}:{}'.format(*server_address))
    http_server.serve_forever()

if __name__ == '__main__':
    main()
