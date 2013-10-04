#!/usr/bin/env python

# Copyright 2013 Google Inc. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
This script will take as an argument either a list of skp files or a
set of directories that contains skp files.  It will then test each
skp file with the `render_pictures` program. If that program either
spits out any unexpected output or doesn't return 0, I will flag that
skp file as problematic.  We then extract all of the embedded images
inside the skp and test each one of them against the
SkImageDecoder::DecodeFile function.  Again, we consider any
extraneous output or a bad return value an error.  In the event of an
error, we retain the image and print out information about the error.
The output (on stdout) is formatted as a csv document.

A copy of each bad image is left in a directory created by
tempfile.mkdtemp().
"""

import glob
import os
import re
import shutil
import subprocess
import sys
import tempfile
import threading

import test_rendering  # skia/trunk/tools.  reuse FindPathToProgram()

USAGE = """
Usage:
    {command} SKP_FILE [SKP_FILES]
    {command} SKP_DIR [SKP_DIRS]\n
Environment variables:
    To run multiple worker threads, set NUM_THREADS.
    To use a different temporary storage location, set TMPDIR.

"""

def execute_program(args, ignores=None):
    """
    Execute a process and waits for it to complete.  Returns all
    output (stderr and stdout) after (optional) filtering.

    @param args is passed into subprocess.Popen().

    @param ignores (optional) is a list of regular expression strings
    that will be ignored in the output.

    @returns a tuple (returncode, output)
    """
    if ignores is None:
        ignores = []
    else:
        ignores = [re.compile(ignore) for ignore in ignores]
    proc = subprocess.Popen(
        args,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT)
    output = ''.join(
        line for line in proc.stdout
        if not any(bool(ignore.match(line)) for ignore in ignores))
    returncode = proc.wait()
    return (returncode, output)


def list_files(paths):
    """
    Accepts a list of directories or filenames on the command line.
    We do not choose to recurse into directories beyond one level.
    """
    class NotAFileException(Exception):
        pass
    for path in paths:
        for globbedpath in glob.iglob(path): # useful on win32
            if os.path.isdir(globbedpath):
                for filename in os.listdir(globbedpath):
                    newpath = os.path.join(globbedpath, filename)
                    if os.path.isfile(newpath):
                        yield newpath
            elif os.path.isfile(globbedpath):
                yield globbedpath
            else:
                raise NotAFileException('{} is not a file'.format(globbedpath))


class BadImageFinder(object):

    def __init__(self, directory=None):
        self.render_pictures = test_rendering.FindPathToProgram(
            'render_pictures')
        self.test_image_decoder = test_rendering.FindPathToProgram(
            'test_image_decoder')
        assert os.path.isfile(self.render_pictures)
        assert os.path.isfile(self.test_image_decoder)
        if directory is None:
            self.saved_image_dir = tempfile.mkdtemp(prefix='skia_skp_test_')
        else:
            assert os.path.isdir(directory)
            self.saved_image_dir = directory
        self.bad_image_count = 0

    def process_files(self, skp_files):
        for path in skp_files:
            self.process_file(path)

    def process_file(self, skp_file):
        assert self.saved_image_dir is not None
        assert os.path.isfile(skp_file)
        args = [self.render_pictures, '--readPath', skp_file]
        ignores = ['^process_in', '^deserializ', '^drawing...', '^Non-defaul']
        returncode, output = execute_program(args, ignores)
        if (returncode == 0) and not output:
            return
        temp_image_dir = tempfile.mkdtemp(prefix='skia_skp_test___')
        args = [ self.render_pictures, '--readPath', skp_file,
                 '--writePath', temp_image_dir, '--writeEncodedImages']
        subprocess.call(args, stderr=open(os.devnull,'w'),
                        stdout=open(os.devnull,'w'))
        for image_name in os.listdir(temp_image_dir):
            image_path = os.path.join(temp_image_dir, image_name)
            assert(os.path.isfile(image_path))
            args = [self.test_image_decoder, image_path]
            returncode, output = execute_program(args, [])
            if (returncode == 0) and not output:
                os.remove(image_path)
                continue
            try:
                shutil.move(image_path, self.saved_image_dir)
            except (shutil.Error,):
                # If this happens, don't stop the entire process,
                # just warn the user.
                os.remove(image_path)
                sys.stderr.write('{0} is a repeat.\n'.format(image_name))
            self.bad_image_count += 1
            if returncode == 2:
                returncode = 'SkImageDecoder::DecodeFile returns false'
            elif returncode == 0:
                returncode = 'extra verbosity'
                assert output
            elif returncode == -11:
                returncode = 'segmentation violation'
            else:
                returncode = 'returncode: {}'.format(returncode)
            output = output.strip().replace('\n',' ').replace('"','\'')
            suffix = image_name[-3:]
            output_line = '"{0}","{1}","{2}","{3}","{4}"\n'.format(
                returncode, suffix, skp_file, image_name, output)
            sys.stdout.write(output_line)
            sys.stdout.flush()
        os.rmdir(temp_image_dir)
        return

def main(main_argv):
    if not main_argv or main_argv[0] in ['-h', '-?', '-help', '--help']:
        sys.stderr.write(USAGE.format(command=__file__))
        return 1
    if 'NUM_THREADS' in os.environ:
        number_of_threads = int(os.environ['NUM_THREADS'])
        if number_of_threads < 1:
            number_of_threads = 1
    else:
        number_of_threads = 1
    os.environ['skia_images_png_suppressDecoderWarnings'] = 'true'
    os.environ['skia_images_jpeg_suppressDecoderWarnings'] = 'true'

    temp_dir = tempfile.mkdtemp(prefix='skia_skp_test_')
    sys.stderr.write('Directory for bad images: {}\n'.format(temp_dir))
    sys.stdout.write('"Error","Filetype","SKP File","Image File","Output"\n')
    sys.stdout.flush()

    finders = [
        BadImageFinder(temp_dir) for index in xrange(number_of_threads)]
    arguments = [[] for index in xrange(number_of_threads)]
    for index, item in enumerate(list_files(main_argv)):
        ## split up the given targets among the worker threads
        arguments[index % number_of_threads].append(item)
    threads = [
        threading.Thread(
            target=BadImageFinder.process_files, args=(finder,argument))
        for finder, argument in zip(finders, arguments)]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join()
    number  = sum(finder.bad_image_count for finder in finders)
    sys.stderr.write('Number of bad images found: {}\n'.format(number))
    return 0

if __name__ == '__main__':
    exit(main(sys.argv[1:]))

#  LocalWords:  skp stdout csv
