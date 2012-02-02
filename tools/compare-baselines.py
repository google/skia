'''
Compares the gm results within the local checkout against those already
committed to the Skia repository. Relies on skdiff to do the low-level
comparison.

Sample usage to compare locally generated gm results against the
checked-in ones:

cd .../trunk
make tools      # or otherwise get a runnable skdiff
python tools/compare-baselines.py gm
# validate that the new images look right

Launch with --help to see more options.


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

# Base URL of SVN repository where we store the checked-in gm results.
SVN_GM_URL = 'http://skia.googlecode.com/svn/trunk/gm'

USAGE_STRING = 'usage: %s [options] <gm basedir>'
OPTION_PATH_TO_SKDIFF = '--path-to-skdiff'

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
    if retval is not 0:
        raise Exception('command [%s] failed' % command)

def Main(options, args):
    """Compare the gm results within the local checkout against those already
    committed to the Skia repository.

    @param options
    @param args
    """
    num_args = len(args)
    if num_args != 1:
        RaiseUsageException()
    gm_basedir = args[0].rstrip(os.sep)

    tempdir_base = tempfile.mkdtemp()

    # Download all checked-in baseline images to a temp directory
    checkedin_dir = os.path.join(tempdir_base, 'checkedin')
    os.mkdir(checkedin_dir)
    svn.Svn(checkedin_dir).Checkout(SVN_GM_URL, '.')

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
    RunCommand('%s %s %s %s' % (options.path_to_skdiff, checkedin_flattened_dir,
                                local_flattened_dir, diff_dir))
    print '\nskdiff results are ready in file://%s/index.html' % diff_dir

def RaiseUsageException():
    raise Exception(USAGE_STRING %  __file__)

if __name__ == '__main__':
    parser = optparse.OptionParser(USAGE_STRING % '%prog')
    parser.add_option(OPTION_PATH_TO_SKDIFF,
                      action='store', type='string',
                      default=os.path.join('out', 'Debug', 'skdiff'),
                      help='path to already-built skdiff tool')
    (options, args) = parser.parse_args()
    Main(options, args)
