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

# modules declared within this same directory
import test_rendering

USAGE_STRING = 'Usage: %s input... expectedDir'
HELP_STRING = '''

Takes input SkPicture files and renders them as PNG files, and then compares
those resulting PNG files against PNG files found in expectedDir.

Each instance of "input" can be either a file (name must end in .skp), or a
directory (in which case this script will process all .skp files within the
directory).
'''

def ModeParse(option, opt_str, value, parser):
    """Parses the --mode option of the commandline.

    The --mode option will either take in three parameters (if tile or
    pow2tile) or a single parameter (otherwise).
    """
    result = [value]
    if value == "tile":
          if (len(parser.rargs) < 2):
              raise optparse.OptionValueError(("--mode tile mising width"
                                               " and/or height parameters"))
          result.extend(parser.rargs[:2])
          del parser.rargs[:2]
    elif value == "pow2tile":
          if (len(parser.rargs) < 2):
              raise optparse.OptionValueError(("--mode pow2tile mising minWidth"
                                               " and/or height parameters"))
          result.extend(parser.rargs[:2])
          del parser.rargs[:2]

    setattr(parser.values, option.dest, result)


def Main(args):
    """Allow other scripts to call this script with fake command-line args.

    @param The commandline argument list
    """
    parser = optparse.OptionParser(USAGE_STRING % '%prog' + HELP_STRING)
    parser.add_option('--render_dir', dest='render_dir',
                    help = ("specify the location to output the rendered files."
                              " Default is a temp directory."))
    parser.add_option('--diff_dir', dest='diff_dir',
                    help = ("specify the location to output the diff files."
                              " Default is a temp directory."))
    parser.add_option('--mode', dest='mode', type='string',
                      action="callback", callback=ModeParse,
                      help = ("specify how rendering is to be done."))
    parser.add_option('--device', dest='device',
                      help = ("specify the device to render to."))

    options, arguments = parser.parse_args(args)

    if (len(arguments) < 3):
        print("Expected at least one input and one ouput folder.")
        parser.print_help()
        sys.exit(-1)

    inputs = arguments[1:-1]
    expected_dir = arguments[-1]

    extra_args = ''

    if (options.mode is not None):
        extra_args += ' --mode %s' % ' '.join(options.mode)

    if (options.device is not None):
        extra_args += ' --device %s' % options.device

    test_rendering.TestRenderSkps(inputs, expected_dir, options.render_dir,
                                  options.diff_dir, 'render_pictures',
                                  extra_args)

if __name__ == '__main__':
    Main(sys.argv)
