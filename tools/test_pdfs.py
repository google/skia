'''
Compares the rendererings of serialized SkPictures to expected images.

Launch with --help to see more information.


Copyright 2012 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''
# common Python modules
import os
import optparse
import sys
import shutil
import tempfile
import test_rendering

USAGE_STRING = 'Usage: %s input... expectedDir'
HELP_STRING = '''

Takes input SkPicture files and renders them as PDF files, and then compares
those resulting PDF files against PDF files found in expectedDir.

Each instance of "input" can be either a file (name must end in .skp), or a
directory (in which case this script will process all .skp files within the
directory).
'''


def Main(args):
    """Allow other scripts to call this script with fake command-line args.

    @param The commandline argument list
    """
    parser = optparse.OptionParser(USAGE_STRING % '%prog' + HELP_STRING)
    parser.add_option('--render_dir', dest='render_dir',
                      help = ('specify the location to output the rendered '
                      'files. Default is a temp directory.'))
    parser.add_option('--diff_dir', dest='diff_dir',
                      help = ('specify the location to output the diff files. '
                      'Default is a temp directory.'))

    options, arguments = parser.parse_args(args)

    if (len(arguments) < 3):
        print("Expected at least one input and one ouput folder.")
        parser.print_help()
        sys.exit(-1)

    inputs = arguments[1:-1]
    expected_dir = arguments[-1]

    test_rendering.TestRenderSkps(inputs, expected_dir, options.render_dir,
                                  options.diff_dir, 'render_pdfs', '')

if __name__ == '__main__':
    Main(sys.argv)

