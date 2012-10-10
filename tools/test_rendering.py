'''
Compares the rendererings of serialized SkPictures to expected result.

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

USAGE_STRING = 'Usage: %s input... expectedDir render_app [reander_app_args]'
HELP_STRING = '''

Compares the renderings of serialized SkPicture files and directories specified
by input with the files in expectedDir. Note, files in directoriers are
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
                      os.path.join(trunk_path, 'out', 'Debug', program),
                      os.path.join(trunk_path, 'out', 'Release',
                                   program + ".exe"),
                      os.path.join(trunk_path, 'out', 'Debug',
                                   program + ".exe")]
    for try_path in possible_paths:
        if os.path.isfile(try_path):
            return try_path
    raise Exception('cannot find %s in paths %s; maybe you need to '
                    'build %s?' % (program, possible_paths, program))


def RenderSkps(inputs, render_dir, render_app, args):
    """Renders the serialized SkPictures.

    Uses the render_pictures program to do the rendering.

    @param inputs the location(s) to read the serlialized SkPictures
    @param render_dir the location to write out the rendered images
    """
    renderer_path = FindPathToProgram(render_app)
    inputs_as_string = " ".join(inputs)
    command = '%s %s %s' % (renderer_path, inputs_as_string, render_dir)

    command += args

    RunCommand(command)


def DiffRenderings(expected_dir, comparison_dir, diff_dir):
    """Diffs the rendered SkPicture files with the baseline files.

    Uses the skdiff program to do the diffing.

    @param expected_dir the location of the baseline images.
    @param comparison_dir the location of the images to comapre with the
           baseline
    @param diff_dir the location to write out the diff results
    """
    skdiff_path = FindPathToProgram('skdiff')
    RunCommand('%s %s %s %s %s' %
               (skdiff_path, expected_dir, comparison_dir, diff_dir,
                '--noprintdirs'))


def Cleanup(render_dir_option, diff_dir_option, render_dir, diff_dir):
    """Deletes any temporary folders and files created.

    @param foo_option The OptionParser parsed render_dir or diff_dir vars.
            If these variables are not passed by user we ended up creating
            temporary directories (render_dir, diff_dir) which we will remove.
    @param render_dir the directory where the rendered images were written
    @param diff_dir the directory where the diff results were written
    """
    if (not render_dir_option):
        if (os.path.isdir(render_dir)):
            shutil.rmtree(render_dir)
    if (not diff_dir_option):
        if (os.path.isdir(diff_dir)):
            shutil.rmtree(diff_dir)

def TestRenderSkps(inputs, expected_dir, render_dir_option, diff_dir_option,
                   render_app, render_args):
    if (render_dir_option):
        render_dir = render_dir_option
    else:
        render_dir = tempfile.mkdtemp()

    if (diff_dir_option):
        diff_dir = diff_dir_option
    else:
        diff_dir = tempfile.mkdtemp()
    try:    
        RenderSkps(inputs, render_dir, render_app, render_args)
        DiffRenderings(expected_dir, render_dir, diff_dir)
    finally:
        Cleanup(render_dir_option, diff_dir_option, render_dir, diff_dir)
