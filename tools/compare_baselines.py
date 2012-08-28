'''
Compares the gm results within the local checkout against those already
committed to the Skia repository.

Launch with --help to see more information.


Copyright 2012 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

# common Python modules
import fnmatch
import optparse
import os
import shutil
import tempfile

# modules declared within this same directory
import svn

USAGE_STRING = 'Usage: %s [options]'
HOWTO_STRING = '''
To update the checked-in baselines across all platforms, follow these steps:

cd .../trunk
svn update
svn stat   # and make sure there are no files awaiting svn commit
make tools BUILDTYPE=Release
python tools/download_baselines.py
python tools/compare_baselines.py
# view compare_baselines output in a browser and make sure it's reasonable
# upload CL for review
# validate that the diffs look right in the review tool
# commit CL

Note that the above instructions will only *update* already-checked-in
baseline images; if you want to check in new baseline images (ones that the
bots have been generating but we don't have a golden master for yet), you need
to use download_baselines.py's --add-new-files option.
'''
HELP_STRING = '''

Compares the gm results within the local checkout against those already
committed to the Skia repository. Relies on skdiff to do the low-level
comparison.

''' + HOWTO_STRING

TRUNK_PATH = os.path.join(os.path.dirname(__file__), os.pardir)

OPTION_GM_BASEDIR = '--gm-basedir'
DEFAULT_GM_BASEDIR = os.path.join(TRUNK_PATH, os.pardir, 'gm-expected')
OPTION_PATH_TO_SKDIFF = '--path-to-skdiff'
# default PATH_TO_SKDIFF is determined at runtime
OPTION_SVN_GM_URL = '--svn-gm-url'
DEFAULT_SVN_GM_URL = 'http://skia.googlecode.com/svn/gm-expected'

def CopyAllFilesAddingPrefix(source_dir, dest_dir, prefix):
    """Copy all files from source_dir into dest_dir, adding prefix to the name
    of each one as we copy it.
    prefixes.

    @param source_dir
    @param dest_dir where to save the copied files
    @param prefix prefix to add to each filename when we make the copy
    """
    all_filenames = os.listdir(source_dir)
    for filename in all_filenames:
        source_path = os.path.join(source_dir, filename)
        if os.path.isdir(source_path):
            print 'skipping %s because it is a directory, not a file' % filename
            continue
        dest_path = os.path.join(dest_dir, '%s%s' % (prefix, filename))
        shutil.copyfile(source_path, dest_path)

def Flatten(source_dir, dest_dir, subdirectory_pattern):
    """Copy all files from matching subdirectories under source_dir into
    dest_dir, flattened into a single directory using subdirectory names as
    prefixes.

    @param source_dir
    @param dest_dir where to save the copied files
    @param subdirectory_pattern only copy files from subdirectories that match
           this Unix-style filename pattern (e.g., 'base-*')
    """
    all_filenames = os.listdir(source_dir)
    matching_filenames = fnmatch.filter(all_filenames, subdirectory_pattern)
    for filename in matching_filenames:
        source_path = os.path.join(source_dir, filename)
        if not os.path.isdir(source_path):
            print 'skipping %s because it is a file, not a directory' % filename
            continue
        print 'flattening directory %s' % source_path
        CopyAllFilesAddingPrefix(source_dir=source_path, dest_dir=dest_dir,
                                 prefix='%s_' % filename)

def RunCommand(command):
    """Run a command, raising an exception if it fails.

    @param command the command as a single string
    """
    print 'running command [%s]...' % command
    retval = os.system(command)
    #if retval is not 0:
    #    raise Exception('command [%s] failed' % command)

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

def CompareBaselines(gm_basedir, path_to_skdiff, svn_gm_url):
    """Compare the gm results within gm_basedir against those already
    committed to the Skia repository.

    @param gm_basedir
    @param path_to_skdiff
    @param svn_gm_url base URL of Subversion repository where we store the
           expected GM results
    """
    # Validate parameters, filling in default values if necessary and possible.
    if not os.path.isdir(gm_basedir):
        raise Exception('cannot find gm_basedir at %s; maybe you need to '
                        'specify the %s option?' % (
                            gm_basedir, OPTION_GM_BASEDIR))
    path_to_skdiff = FindPathToSkDiff(path_to_skdiff)

    tempdir_base = tempfile.mkdtemp()

    # Download all checked-in baseline images to a temp directory
    checkedin_dir = os.path.join(tempdir_base, 'checkedin')
    os.mkdir(checkedin_dir)
    svn.Svn(checkedin_dir).Checkout(svn_gm_url, '.')

    # Flatten those checked-in baseline images into checkedin_flattened_dir
    checkedin_flattened_dir = os.path.join(tempdir_base, 'checkedin_flattened')
    os.mkdir(checkedin_flattened_dir)
    Flatten(source_dir=checkedin_dir, dest_dir=checkedin_flattened_dir,
            subdirectory_pattern='base-*')

    # Flatten the local baseline images into local_flattened_dir
    local_flattened_dir = os.path.join(tempdir_base, 'local_flattened')
    os.mkdir(local_flattened_dir)
    Flatten(source_dir=gm_basedir, dest_dir=local_flattened_dir,
            subdirectory_pattern='base-*')

    # Run skdiff to compare checkedin_flattened_dir against local_flattened_dir
    diff_dir = os.path.join(tempdir_base, 'diffs')
    os.mkdir(diff_dir)
    RunCommand('%s %s %s %s' % (path_to_skdiff, checkedin_flattened_dir,
                                local_flattened_dir, diff_dir))
    print '\nskdiff results are ready in file://%s/index.html' % diff_dir
    # TODO(epoger): delete tempdir_base tree to clean up after ourselves (but
    # not before the user gets a chance to examine the results), and/or
    # allow user to specify a different directory to write into?

def RaiseUsageException():
    raise Exception('%s\nRun with --help for more detail.' % (
        USAGE_STRING % __file__))

def Main(options, args):
    """Allow other scripts to call this script with fake command-line args.
    """
    num_args = len(args)
    if num_args != 0:
        RaiseUsageException()
    CompareBaselines(gm_basedir=options.gm_basedir,
                     path_to_skdiff=options.path_to_skdiff,
                     svn_gm_url=options.svn_gm_url)

if __name__ == '__main__':
    parser = optparse.OptionParser(USAGE_STRING % '%prog' + HELP_STRING)
    parser.add_option(OPTION_GM_BASEDIR,
                      action='store', type='string', default=DEFAULT_GM_BASEDIR,
                      help='path to root of locally stored baseline images '
                      'to compare against those checked into the svn repo; '
                      'defaults to "%s"' % DEFAULT_GM_BASEDIR)
    parser.add_option(OPTION_PATH_TO_SKDIFF,
                      action='store', type='string', default=None,
                      help='path to already-built skdiff tool; if not set, '
                      'will search for it in typical directories near this '
                      'script')
    parser.add_option(OPTION_SVN_GM_URL,
                      action='store', type='string', default=DEFAULT_SVN_GM_URL,
                      help='URL of SVN repository within which we store the '
                      'expected GM baseline images; defaults to "%s"' %
                      DEFAULT_SVN_GM_URL)
    (options, args) = parser.parse_args()
    Main(options, args)
