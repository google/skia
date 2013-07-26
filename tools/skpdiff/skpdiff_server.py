#!/usr/bin/python
# -*- coding: utf-8 -*-

from __future__ import print_function
import argparse
import BaseHTTPServer
import json
import os
import os.path
import re
import sys
import tempfile
import urllib2

# Grab the script path because that is where all the static assets are
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Find the tools directory for python imports
TOOLS_DIR = os.path.dirname(SCRIPT_DIR)

# Find the root of the skia trunk for finding skpdiff binary
SKIA_ROOT_DIR = os.path.dirname(TOOLS_DIR)

# Find the default location of gm expectations
DEFAULT_GM_EXPECTATIONS_DIR = os.path.join(SKIA_ROOT_DIR, 'expectations', 'gm')

# Imports from within Skia
if TOOLS_DIR not in sys.path:
    sys.path.append(TOOLS_DIR)
GM_DIR = os.path.join(SKIA_ROOT_DIR, 'gm')
if GM_DIR not in sys.path:
    sys.path.append(GM_DIR)
import gm_json
import jsondiff

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


IMAGE_FILENAME_RE = re.compile(gm_json.IMAGE_FILENAME_PATTERN)

SKPDIFF_INVOKE_FORMAT = '{} --jsonp=false -o {} -f {} {}'


def get_skpdiff_path(user_path=None):
    """Find the skpdiff binary.

    @param user_path If none, searches in Release and Debug out directories of
           the skia root. If set, checks that the path is a real file and
           returns it.
    """
    skpdiff_path = None
    possible_paths = []

    # Use the user given path, or try out some good default paths.
    if user_path:
        possible_paths.append(user_path)
    else:
        possible_paths.append(os.path.join(SKIA_ROOT_DIR, 'out',
                                           'Release', 'skpdiff'))
        possible_paths.append(os.path.join(SKIA_ROOT_DIR, 'out',
                                           'Debug', 'skpdiff'))
    # Use the first path that actually points to the binary
    for possible_path in possible_paths:
        if os.path.isfile(possible_path):
            skpdiff_path = possible_path
            break

    # If skpdiff was not found, print out diagnostic info for the user.
    if skpdiff_path is None:
        print('Could not find skpdiff binary. Either build it into the ' +
              'default directory, or specify the path on the command line.')
        print('skpdiff paths tried:')
        for possible_path in possible_paths:
            print('   ', possible_path)
    return skpdiff_path


def download_file(url, output_path):
    """Download the file at url and place it in output_path"""
    reader = urllib2.urlopen(url)
    with open(output_path, 'wb') as writer:
        writer.write(reader.read())


def download_gm_image(image_name, image_path, hash_val):
    """Download the gm result into the given path.

    @param image_name The GM file name, for example imageblur_gpu.png.
    @param image_path Path to place the image.
    @param hash_val   The hash value of the image.
    """

    # Seperate the test name from a image name
    image_match = IMAGE_FILENAME_RE.match(image_name)
    test_name = image_match.group(1)

    # Calculate the URL of the requested image
    image_url = gm_json.CreateGmActualUrl(
        test_name, gm_json.JSONKEY_HASHTYPE_BITMAP_64BITMD5, hash_val)

    # Download the image as requested
    download_file(image_url, image_path)


def download_changed_images(expectations_dir, expected_name, updated_name,
                            expected_image_dir, actual_image_dir):

    """Download the expected and actual GMs that changed into the given paths.
    Determining what changed will be done by comparing each expected_name JSON
    results file to its corresponding updated_name JSON results file if it
    exists.

    @param expectations_dir   The directory to traverse for results files. This
           should resmble expectations/gm in the Skia trunk.
    @param expected_name      The name of the expected result files. These are
           in the format of expected-results.json.
    @param updated_name       The name of the updated expected result files.
           Normally this matches --expectations-filename-output for the
           rebaseline.py tool.
    @param expected_image_dir The directory to place downloaded expected images
           into.
    @param actual_image_dir   The directory to place downloaded actual images
           into.
    """

    differ = jsondiff.GMDiffer()

    # Look through expectations for hashes that changed
    for root, dirs, files in os.walk(expectations_dir):
        for expectation_file in files:
            # There are many files in the expectations directory. We only care
            # about expected results.
            if expectation_file != expected_name:
                continue

            # Get the name of the results file, and be sure there is an updated
            # result to compare against. If there is not, there is no point in
            # diffing this device.
            expected_file_path = os.path.join(root, expected_name)
            updated_file_path = os.path.join(root, updated_name)
            if not os.path.isfile(updated_file_path):
                continue

            # Find all expectations that did not match.
            expected_diff = differ.GenerateDiffDict(expected_file_path,
                                                    updated_file_path)

            # The name of the device corresponds to the name of the folder we
            # are in.
            device_name = os.path.basename(root)

            # Create name prefixes to store the devices old and new GM results
            expected_image_prefix = os.path.join(expected_image_dir,
                                                 device_name) + '-'

            actual_image_prefix = os.path.join(actual_image_dir,
                                               device_name) + '-'

            # Download each image that had a differing result
            for image_name, hashes in expected_diff.iteritems():
                print('Downloading', image_name, 'for device', device_name)
                download_gm_image(image_name,
                                  expected_image_prefix + image_name,
                                  hashes['old'])
                download_gm_image(image_name,
                                  actual_image_prefix + image_name,
                                  hashes['new'])


def get_image_set_from_skpdiff(skpdiff_records):
    """Get the set of all images references in the given records.

    @param skpdiff_records An array of records, which are dictionary objects.
    """
    expected_set = frozenset([r['baselinePath'] for r in skpdiff_records])
    actual_set = frozenset([r['testPath'] for r in skpdiff_records])
    return expected_set | actual_set


class SkPDiffHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def send_file(self, file_path):
        # Grab the extension if there is one
        extension = os.path.splitext(file_path)[1]
        if len(extension) >= 1:
            extension = extension[1:]

        # Determine the MIME type of the file from its extension
        mime_type = MIME_TYPE_MAP.get(extension, MIME_TYPE_MAP[''])

        # Open the file and send it over HTTP
        if os.path.isfile(file_path):
            with open(file_path, 'rb') as sending_file:
                self.send_response(200)
                self.send_header('Content-type', mime_type)
                self.end_headers()
                self.wfile.write(sending_file.read())
        else:
            self.send_error(404)

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
        # Simple rewrite rule of the root path to 'viewer.html'
        if self.path == '' or self.path == '/':
            self.path = '/viewer.html'

        # The [1:] chops off the leading '/'
        file_path = self.path[1:]

        # Handle skpdiff_output.json manually because it is was processed by the
        # server when it was started and does not exist as a file.
        if file_path == 'skpdiff_output.json':
            self.send_response(200)
            self.send_header('Content-type', MIME_TYPE_MAP['json'])
            self.end_headers()
            self.wfile.write(self.server.skpdiff_output_json)
            return

        # Attempt to send static asset files first.
        if self.serve_if_in_dir(SCRIPT_DIR, file_path):
            return

        # WARNING: Serving any file the user wants is incredibly insecure. Its
        # redeeming quality is that we only serve gm files on a white list.
        if self.path in self.server.image_set:
            self.send_file(self.path)
            return

        # If no file to send was found, just give the standard 404
        self.send_error(404)


def run_server(skpdiff_output_path, port=8080):
    # Preload the skpdiff results file. This is so we can perform some
    # processing on it.
    skpdiff_output_json = ''
    with open(skpdiff_output_path, 'rb') as records_file:
        skpdiff_output_json = records_file.read()

    # It's important to parse the results file so that we can make a set of
    # images that the web page might request.
    skpdiff_records = json.loads(skpdiff_output_json)['records']
    image_set = get_image_set_from_skpdiff(skpdiff_records)

    # Add JSONP padding to the JSON because the web page expects it. It expects
    # it because it was designed to run with or without a web server. Without a
    # web server, the only way to load JSON is with JSONP.
    skpdiff_output_json = 'var SkPDiffRecords = ' + skpdiff_output_json

    # Do not bind to interfaces other than localhost because the server will
    # attempt to serve files relative to the root directory as a last resort
    # before 404ing. This means all of your files can be accessed from this
    # server, so DO NOT let this server listen to anything but localhost.
    server_address = ('127.0.0.1', port)
    http_server = BaseHTTPServer.HTTPServer(server_address, SkPDiffHandler)
    http_server.image_set = image_set
    http_server.skpdiff_output_json = skpdiff_output_json
    print('Navigate thine browser to: http://{}:{}'.format(*server_address))
    http_server.serve_forever()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', '-p', metavar='PORT',
                        type=int,
                        default=8080,
                        help='port to bind the server to; ' +
                        'defaults to %(default)s',
                        )

    parser.add_argument('--expectations-dir', metavar='EXPECTATIONS_DIR',
                        default=DEFAULT_GM_EXPECTATIONS_DIR,
                        help='path to the gm expectations; ' +
                        'defaults to %(default)s'
                        )

    parser.add_argument('--expected',
                        metavar='EXPECTATIONS_FILE_NAME',
                        default='expected-results.json',
                        help='the file name of the expectations JSON; ' +
                        'defaults to %(default)s'
                        )

    parser.add_argument('--updated',
                        metavar='UPDATED_FILE_NAME',
                        default='updated-results.json',
                        help='the file name of the updated expectations JSON;' +
                        ' defaults to %(default)s'
                        )

    parser.add_argument('--skpdiff-path', metavar='SKPDIFF_PATH',
                        default=None,
                        help='the path to the skpdiff binary to use; ' +
                        'defaults to out/Release/skpdiff or out/Default/skpdiff'
                        )

    args = vars(parser.parse_args())  # Convert args into a python dict

    # Make sure we have access to an skpdiff binary
    skpdiff_path = get_skpdiff_path(args['skpdiff_path'])
    if skpdiff_path is None:
        sys.exit(1)

    # Create a temporary file tree that makes sense for skpdiff.to operate on
    image_output_dir = tempfile.mkdtemp('skpdiff')
    expected_image_dir = os.path.join(image_output_dir, 'expected')
    actual_image_dir = os.path.join(image_output_dir, 'actual')
    os.mkdir(expected_image_dir)
    os.mkdir(actual_image_dir)

    # Print out the paths of things for easier debugging
    print('script dir         :', SCRIPT_DIR)
    print('tools dir          :', TOOLS_DIR)
    print('root dir           :', SKIA_ROOT_DIR)
    print('expectations dir   :', args['expectations_dir'])
    print('skpdiff path       :', skpdiff_path)
    print('tmp dir            :', image_output_dir)
    print('expected image dir :', expected_image_dir)
    print('actual image dir   :', actual_image_dir)

    # Download expected and actual images that differed into the temporary file
    # tree.
    download_changed_images(args['expectations_dir'],
                            args['expected'],   args['updated'],
                            expected_image_dir, actual_image_dir)

    # Invoke skpdiff with our downloaded images and place its results in the
    # temporary directory.
    skpdiff_output_path = os.path.join(image_output_dir, 'skpdiff_output.json')
    skpdiff_cmd = SKPDIFF_INVOKE_FORMAT.format(skpdiff_path,
                                               skpdiff_output_path,
                                               expected_image_dir,
                                               actual_image_dir)
    os.system(skpdiff_cmd)

    run_server(skpdiff_output_path, port=args['port'])

if __name__ == '__main__':
    main()
