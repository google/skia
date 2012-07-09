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

USAGE_STRING = 'Usage: %s input... expectedDir'
HELP_STRING = '''

Compares the renderings of serialized SkPicture files and directories specified
by input with the images in expectedDir. Note, files in directoriers are
expected to end with .skp.
'''

def RunCommand(command):
    """Run a command.

    @param command the command as a single string
    """
    print 'running command [%s]...' % command
    os.system(command)


def FindPathToProgram(program):
    """Return path to an existing program binary, or raise an exception if we
    cannot find one.

    @param program the name of the program that is being looked for
    """
    trunk_path = os.path.abspath(os.path.join(os.path.dirname(__file__),
                                              os.pardir))
    possible_paths = [os.path.join(trunk_path, 'out', 'Release', program),
                      os.path.join(trunk_path, 'out', 'Debug', program)]
    for try_path in possible_paths:
        if os.path.isfile(try_path):
            return try_path
    raise Exception('cannot find %s in paths %s; maybe you need to '
                    'build %s?' % (program, possible_paths, program))


def RenderImages(inputs, render_dir):
    """Renders the serialized SkPictures.

    Uses the render_pictures program to do the rendering.

    @param inputs the location(s) to read the serlialized SkPictures
    @param render_dir the location to write out the rendered images
    """
    renderer_path = FindPathToProgram('render_pictures')
    inputs_as_string = " ".join(inputs)
    RunCommand('%s %s %s' % (renderer_path, inputs_as_string, render_dir))


def DiffImages(expected_dir, comparison_dir, diff_dir):
    """Diffs the rendered SkPicture images with the baseline images.

    Uses the skdiff program to do the diffing.

    @param expected_dir the location of the baseline images.
    @param comparison_dir the location of the images to comapre with the
           baseline
    @param diff_dir the location to write out the diff results
    """
    skdiff_path = FindPathToProgram('skdiff')
    RunCommand('%s %s %s %s' %
               (skdiff_path, expected_dir, comparison_dir, diff_dir))


def Cleanup(options, render_dir, diff_dir):
    """Deletes any temporary folders and files created.

    @param options The OptionParser object that parsed if render_dir or diff_dir
           was set
    @param render_dir the directory where the rendered images were written
    @param diff_dir the directory where the diff results were written
    """
    if (not options.render_dir):
        if (os.path.isdir(render_dir)):
            shutil.rmtree(render_dir)
    if (not options.diff_dir):
        if (os.path.isdir(diff_dir)):
            shutil.rmtree(diff_dir)


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

    options, arguments = parser.parse_args(args)

    if (len(arguments) < 3):
        print("Expected at least one input and one ouput folder.")
        parser.print_help()
        sys.exit(-1)

    inputs = arguments[1:-1]
    expected_dir = arguments[-1]

    if (options.render_dir):
        render_dir = options.render_dir
    else:
        render_dir = tempfile.mkdtemp()

    if (options.diff_dir):
        diff_dir = options.diff_dir
    else:
        diff_dir = tempfile.mkdtemp()

    try:
        RenderImages(inputs, render_dir)
        DiffImages(expected_dir, render_dir, diff_dir)
    finally:
        Cleanup(options, render_dir, diff_dir)

if __name__ == '__main__':
    Main(sys.argv)
