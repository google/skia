#!/usr/bin/python
# -*- coding: utf-8 -*-

from __future__ import print_function
import argparse
import BaseHTTPServer
import json
import os
import os.path
import re
import subprocess
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
                                           'Release', 'skpdiff.exe'))
        possible_paths.append(os.path.join(SKIA_ROOT_DIR, 'out',
                                           'Debug', 'skpdiff'))
        possible_paths.append(os.path.join(SKIA_ROOT_DIR, 'out',
                                           'Debug', 'skpdiff.exe'))
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
    if hash_val is None:
        return

    # Separate the test name from a image name
    image_match = IMAGE_FILENAME_RE.match(image_name)
    test_name = image_match.group(1)

    # Calculate the URL of the requested image
    image_url = gm_json.CreateGmActualUrl(
        test_name, gm_json.JSONKEY_HASHTYPE_BITMAP_64BITMD5, hash_val)

    # Download the image as requested
    download_file(image_url, image_path)


def get_image_set_from_skpdiff(skpdiff_records):
    """Get the set of all images references in the given records.

    @param skpdiff_records An array of records, which are dictionary objects.
    """
    expected_set = frozenset([r['baselinePath'] for r in skpdiff_records])
    actual_set = frozenset([r['testPath'] for r in skpdiff_records])
    return expected_set | actual_set


def set_expected_hash_in_json(expected_results_json, image_name, hash_value):
    """Set the expected hash for the object extracted from
    expected-results.json. Note that this only work with bitmap-64bitMD5 hash
    types.

    @param expected_results_json The Python dictionary with the results to
    modify.
    @param image_name            The name of the image to set the hash of.
    @param hash_value            The hash to set for the image.
    """
    expected_results = expected_results_json[gm_json.JSONKEY_EXPECTEDRESULTS]

    if image_name in expected_results:
        expected_results[image_name][gm_json.JSONKEY_EXPECTEDRESULTS_ALLOWEDDIGESTS][0][1] = hash_value
    else:
        expected_results[image_name] = {
            gm_json.JSONKEY_EXPECTEDRESULTS_ALLOWEDDIGESTS:
            [
                [
                    gm_json.JSONKEY_HASHTYPE_BITMAP_64BITMD5,
                    hash_value
                ]
            ]
        }


def get_head_version(path):
    """Get the version of the file at the given path stored inside the HEAD of
    the git repository. It is returned as a string.

    @param path The path of the file whose HEAD is returned. It is assumed the
    path is inside a git repo rooted at SKIA_ROOT_DIR.
    """

    # git-show will not work with absolute paths. This ensures we give it a path
    # relative to the skia root. This path also has to use forward slashes, even
    # on windows.
    git_path = os.path.relpath(path, SKIA_ROOT_DIR).replace('\\', '/')
    git_show_proc = subprocess.Popen(['git', 'show', 'HEAD:' + git_path],
                                     stdout=subprocess.PIPE)

    # When invoked outside a shell, git will output the last committed version
    # of the file directly to stdout.
    git_version_content, _ = git_show_proc.communicate()
    return git_version_content


class GMInstance:
    """Information about a GM test result on a specific device:
     - device_name = the name of the device that rendered it
     - image_name = the GM test name and config
     - expected_hash = the current expected hash value
     - actual_hash = the actual hash value
     - is_rebaselined = True if actual_hash is what is currently in the expected
                        results file, False otherwise.
    """
    def __init__(self,
                 device_name, image_name,
                 expected_hash, actual_hash,
                 is_rebaselined):
        self.device_name = device_name
        self.image_name = image_name
        self.expected_hash = expected_hash
        self.actual_hash = actual_hash
        self.is_rebaselined = is_rebaselined


class ExpectationsManager:
    def __init__(self, expectations_dir, expected_name, updated_name,
                 skpdiff_path):
        """
        @param expectations_dir   The directory to traverse for results files.
               This should resemble expectations/gm in the Skia trunk.
        @param expected_name      The name of the expected result files. These
               are in the format of expected-results.json.
        @param updated_name       The name of the updated expected result files.
               Normally this matches --expectations-filename-output for the
               rebaseline.py tool.
        @param skpdiff_path       The path used to execute the skpdiff command.
        """
        self._expectations_dir = expectations_dir
        self._expected_name = expected_name
        self._updated_name = updated_name
        self._skpdiff_path = skpdiff_path
        self._generate_gm_comparison()

    def _generate_gm_comparison(self):
        """Generate all the data needed to compare GMs:
           - determine which GMs changed
           - download the changed images
           - compare them with skpdiff
        """

        # Get the expectations and compare them with actual hashes
        self._get_expectations()


        # Create a temporary file tree that makes sense for skpdiff to operate
        # on. We take the realpath of the new temp directory because some OSs
        # (*cough* osx) put the temp directory behind a symlink that gets
        # resolved later down the pipeline and breaks the image map.
        image_output_dir = os.path.realpath(tempfile.mkdtemp('skpdiff'))
        expected_image_dir = os.path.join(image_output_dir, 'expected')
        actual_image_dir = os.path.join(image_output_dir, 'actual')
        os.mkdir(expected_image_dir)
        os.mkdir(actual_image_dir)

        # Download expected and actual images that differed into the temporary
        # file tree.
        self._download_expectation_images(expected_image_dir, actual_image_dir)

        # Invoke skpdiff with our downloaded images and place its results in the
        # temporary directory.
        self._skpdiff_output_path = os.path.join(image_output_dir,
                                                'skpdiff_output.json')
        skpdiff_cmd = SKPDIFF_INVOKE_FORMAT.format(self._skpdiff_path,
                                                   self._skpdiff_output_path,
                                                   expected_image_dir,
                                                   actual_image_dir)
        os.system(skpdiff_cmd)
        self._load_skpdiff_output()


    def _get_expectations(self):
        """Fills self._expectations with GMInstance objects for each test whose
        expectation is different between the following two files:
         - the local filesystem's updated results file
         - git's head version of the expected results file
        """
        differ = jsondiff.GMDiffer()
        self._expectations = []
        for root, dirs, files in os.walk(self._expectations_dir):
            for expectation_file in files:
                # There are many files in the expectations directory. We only
                # care about expected results.
                if expectation_file != self._expected_name:
                    continue

                # Get the name of the results file, and be sure there is an
                # updated result to compare against. If there is not, there is
                # no point in diffing this device.
                expected_file_path = os.path.join(root, self._expected_name)
                updated_file_path = os.path.join(root, self._updated_name)
                if not os.path.isfile(updated_file_path):
                    continue

                # Always get the expected results from git because we may have
                # changed them in a previous instance of the server.
                expected_contents = get_head_version(expected_file_path)
                updated_contents = None
                with open(updated_file_path, 'rb') as updated_file:
                    updated_contents = updated_file.read()

                # Read the expected results on disk to determine what we've
                # already rebaselined.
                commited_contents = None
                with open(expected_file_path, 'rb') as expected_file:
                    commited_contents = expected_file.read()

                # Find all expectations that did not match.
                expected_diff = differ.GenerateDiffDictFromStrings(
                    expected_contents,
                    updated_contents)

                # Generate a set of images that have already been rebaselined
                # onto disk.
                rebaselined_diff = differ.GenerateDiffDictFromStrings(
                    expected_contents,
                    commited_contents)

                rebaselined_set = set(rebaselined_diff.keys())

                # The name of the device corresponds to the name of the folder
                # we are in.
                device_name = os.path.basename(root)

                # Store old and new versions of the expectation for each GM
                for image_name, hashes in expected_diff.iteritems():
                    self._expectations.append(
                        GMInstance(device_name, image_name,
                                   hashes['old'], hashes['new'],
                                   image_name in rebaselined_set))

    def _load_skpdiff_output(self):
        """Loads the results of skpdiff and annotates them with whether they
        have already been rebaselined or not. The resulting data is store in
        self.skpdiff_records."""
        self.skpdiff_records = None
        with open(self._skpdiff_output_path, 'rb') as skpdiff_output_file:
            self.skpdiff_records = json.load(skpdiff_output_file)['records']
            for record in self.skpdiff_records:
                record['isRebaselined'] = self.image_map[record['baselinePath']][1].is_rebaselined


    def _download_expectation_images(self, expected_image_dir, actual_image_dir):
        """Download the expected and actual images for the _expectations array.

        @param expected_image_dir The directory to download expected images
               into.
        @param actual_image_dir   The directory to download actual images into.
        """
        image_map = {}

        # Look through expectations and download their images.
        for expectation in self._expectations:
            # Build appropriate paths to download the images into.
            expected_image_path = os.path.join(expected_image_dir,
                                               expectation.device_name + '-' +
                                               expectation.image_name)

            actual_image_path = os.path.join(actual_image_dir,
                                             expectation.device_name + '-' +
                                             expectation.image_name)

            print('Downloading %s for device %s' % (
                expectation.image_name, expectation.device_name))

            # Download images
            download_gm_image(expectation.image_name,
                              expected_image_path,
                              expectation.expected_hash)

            download_gm_image(expectation.image_name,
                              actual_image_path,
                              expectation.actual_hash)

            # Annotate the expectations with where the images were downloaded
            # to.
            expectation.expected_image_path = expected_image_path
            expectation.actual_image_path = actual_image_path

            # Map the image paths back to the expectations.
            image_map[expected_image_path] = (False, expectation)
            image_map[actual_image_path] = (True, expectation)

        self.image_map = image_map

    def _set_expected_hash(self, device_name, image_name, hash_value):
        """Set the expected hash for the image of the given device. This always
        writes directly to the expected results file of the given device

        @param device_name The name of the device to write the hash to.
        @param image_name  The name of the image whose hash to set.
        @param hash_value  The value of the hash to set.
        """

        # Retrieve the expected results file as it is in the working tree
        json_path = os.path.join(self._expectations_dir, device_name,
                                 self._expected_name)
        expectations = gm_json.LoadFromFile(json_path)

        # Set the specified hash.
        set_expected_hash_in_json(expectations, image_name, hash_value)

        # Write it out to disk using gm_json to keep the formatting consistent.
        gm_json.WriteToFile(expectations, json_path)

    def commit_rebaselines(self, rebaselines):
        """Sets the expected results file to use the hashes of the images in
        the rebaselines list. If a expected result image is not in rebaselines
        at all, the old hash will be used.

        @param rebaselines A list of image paths to use the hash of.
        """
        # Reset all expectations to their old hashes because some of them may
        # have been set to the new hash by a previous call to this function.
        for expectation in self._expectations:
            expectation.is_rebaselined = False
            self._set_expected_hash(expectation.device_name,
                                    expectation.image_name,
                                    expectation.expected_hash)

        # Take all the images to rebaseline
        for image_path in rebaselines:
            # Get the metadata about the image at the path.
            is_actual, expectation = self.image_map[image_path]

            expectation.is_rebaselined = is_actual
            expectation_hash = expectation.actual_hash if is_actual else\
                               expectation.expected_hash

            # Write out that image's hash directly to the expected results file.
            self._set_expected_hash(expectation.device_name,
                                    expectation.image_name,
                                    expectation_hash)

        self._load_skpdiff_output()


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

            # Add JSONP padding to the JSON because the web page expects it. It
            # expects it because it was designed to run with or without a web
            # server. Without a web server, the only way to load JSON is with
            # JSONP.
            skpdiff_records = self.server.expectations_manager.skpdiff_records
            self.wfile.write('var SkPDiffRecords = ')
            json.dump({'records': skpdiff_records}, self.wfile)
            self.wfile.write(';')
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

    def do_POST(self):
        if self.path == '/commit_rebaselines':
            content_length = int(self.headers['Content-length'])
            request_data = json.loads(self.rfile.read(content_length))
            rebaselines = request_data['rebaselines']
            self.server.expectations_manager.commit_rebaselines(rebaselines)
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write('{"success":true}')
            return

        # If the we have no handler for this path, give em' the 404
        self.send_error(404)


def run_server(expectations_manager, port=8080):
    # It's important to parse the results file so that we can make a set of
    # images that the web page might request.
    skpdiff_records = expectations_manager.skpdiff_records
    image_set = get_image_set_from_skpdiff(skpdiff_records)

    # Do not bind to interfaces other than localhost because the server will
    # attempt to serve files relative to the root directory as a last resort
    # before 404ing. This means all of your files can be accessed from this
    # server, so DO NOT let this server listen to anything but localhost.
    server_address = ('127.0.0.1', port)
    http_server = BaseHTTPServer.HTTPServer(server_address, SkPDiffHandler)
    http_server.image_set = image_set
    http_server.expectations_manager = expectations_manager
    print('Navigate thine browser to: http://{}:{}/'.format(*server_address))
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

    # Print out the paths of things for easier debugging
    print('script dir         :', SCRIPT_DIR)
    print('tools dir          :', TOOLS_DIR)
    print('root dir           :', SKIA_ROOT_DIR)
    print('expectations dir   :', args['expectations_dir'])
    print('skpdiff path       :', skpdiff_path)

    expectations_manager = ExpectationsManager(args['expectations_dir'],
                                               args['expected'],
                                               args['updated'],
                                               skpdiff_path)

    run_server(expectations_manager, port=args['port'])

if __name__ == '__main__':
    main()
