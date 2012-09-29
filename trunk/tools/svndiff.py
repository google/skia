'''
Generates a visual diff of all pending changes in the local SVN checkout.

Launch with --help to see more information.


Copyright 2012 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

# common Python modules
import optparse
import os
import re
import shutil
import tempfile

# modules declared within this same directory
import svn

USAGE_STRING = 'Usage: %s [options]'
HELP_STRING = '''

Generates a visual diff of all pending changes in the local SVN checkout.

This includes a list of all files that have been added, deleted, or modified
(as far as SVN knows about).  For any image modifications, pixel diffs will
be generated.

'''

TRUNK_PATH = os.path.join(os.path.dirname(__file__), os.pardir)

OPTION_DEST_DIR = '--dest-dir'
# default DEST_DIR is determined at runtime
OPTION_PATH_TO_SKDIFF = '--path-to-skdiff'
# default PATH_TO_SKDIFF is determined at runtime

def RunCommand(command):
    """Run a command, raising an exception if it fails.

    @param command the command as a single string
    """
    print 'running command [%s]...' % command
    retval = os.system(command)
    if retval is not 0:
        raise Exception('command [%s] failed' % command)

def FindPathToSkDiff(user_set_path=None):
    """Return path to an existing skdiff binary, or raise an exception if we
    cannot find one.

    @param user_set_path if None, the user did not specify a path, so look in
           some likely places; otherwise, only check at this path
    """
    if user_set_path is not None:
        if os.path.isfile(user_set_path):
            return user_set_path
        raise Exception('unable to find skdiff at user-set path %s' %
                        user_set_path)
    trunk_path = os.path.join(os.path.dirname(__file__), os.pardir)
    possible_paths = [os.path.join(trunk_path, 'out', 'Release', 'skdiff'),
                      os.path.join(trunk_path, 'out', 'Debug', 'skdiff')]
    for try_path in possible_paths:
        if os.path.isfile(try_path):
            return try_path
    raise Exception('cannot find skdiff in paths %s; maybe you need to '
                    'specify the %s option or build skdiff?' % (
                        possible_paths, OPTION_PATH_TO_SKDIFF))

def SvnDiff(path_to_skdiff, dest_dir):
    """Generates a visual diff of all pending changes in the local SVN checkout.

    @param path_to_skdiff
    @param dest_dir existing directory within which to write results
    """
    # Validate parameters, filling in default values if necessary and possible.
    path_to_skdiff = FindPathToSkDiff(path_to_skdiff)
    if not dest_dir:
        dest_dir = tempfile.mkdtemp()

    # Prepare temporary directories.
    modified_flattened_dir = os.path.join(dest_dir, 'modified_flattened')
    original_flattened_dir = os.path.join(dest_dir, 'original_flattened')
    diff_dir = os.path.join(dest_dir, 'diffs')
    for dir in [modified_flattened_dir, original_flattened_dir, diff_dir] :
        shutil.rmtree(dir, ignore_errors=True)
        os.mkdir(dir)

    # Get a list of all locally modified (including added/deleted) files,
    # descending subdirectories.
    svn_repo = svn.Svn('.')
    modified_file_paths = svn_repo.GetFilesWithStatus(
        svn.STATUS_ADDED | svn.STATUS_DELETED | svn.STATUS_MODIFIED)

    # For each modified file:
    # 1. copy its current contents into modified_flattened_dir
    # 2. copy its original contents into original_flattened_dir
    for modified_file_path in modified_file_paths:
        dest_filename = re.sub(os.sep, '__', modified_file_path)
        # If the file had STATUS_DELETED, it won't exist anymore...
        if os.path.isfile(modified_file_path):
            shutil.copyfile(modified_file_path,
                            os.path.join(modified_flattened_dir, dest_filename))
        svn_repo.ExportBaseVersionOfFile(
            modified_file_path,
            os.path.join(original_flattened_dir, dest_filename))

    # Run skdiff: compare original_flattened_dir against modified_flattened_dir
    RunCommand('%s %s %s %s' % (path_to_skdiff, original_flattened_dir,
                                modified_flattened_dir, diff_dir))
    print '\nskdiff results are ready in file://%s/index.html' % diff_dir

def RaiseUsageException():
    raise Exception('%s\nRun with --help for more detail.' % (
        USAGE_STRING % __file__))

def Main(options, args):
    """Allow other scripts to call this script with fake command-line args.
    """
    num_args = len(args)
    if num_args != 0:
        RaiseUsageException()
    SvnDiff(path_to_skdiff=options.path_to_skdiff, dest_dir=options.dest_dir)

if __name__ == '__main__':
    parser = optparse.OptionParser(USAGE_STRING % '%prog' + HELP_STRING)
    parser.add_option(OPTION_DEST_DIR,
                      action='store', type='string', default=None,
                      help='existing directory within which to write results; '
                      'if not set, will create a temporary directory which '
                      'will remain in place after this script completes')
    parser.add_option(OPTION_PATH_TO_SKDIFF,
                      action='store', type='string', default=None,
                      help='path to already-built skdiff tool; if not set, '
                      'will search for it in typical directories near this '
                      'script')
    (options, args) = parser.parse_args()
    Main(options, args)
