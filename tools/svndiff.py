#!/usr/bin/python
'''
Copyright 2012 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

'''
Generates a visual diff of all pending changes in the local SVN (or git!)
checkout.

Launch with --help to see more information.

TODO(epoger): Now that this tool supports either git or svn, rename it.
TODO(epoger): Fix indentation in this file (2-space indents, not 4-space).
'''

# common Python modules
import optparse
import os
import posixpath
import re
import shutil
import subprocess
import sys
import tempfile
import urllib2

# Imports from within Skia
#
# We need to add the 'gm' directory, so that we can import gm_json.py within
# that directory.  That script allows us to parse the actual-results.json file
# written out by the GM tool.
# Make sure that the 'gm' dir is in the PYTHONPATH, but add it at the *end*
# so any dirs that are already in the PYTHONPATH will be preferred.
#
# This assumes that the 'gm' directory has been checked out as a sibling of
# the 'tools' directory containing this script, which will be the case if
# 'trunk' was checked out as a single unit.
GM_DIRECTORY = os.path.realpath(
    os.path.join(os.path.dirname(os.path.dirname(__file__)), 'gm'))
if GM_DIRECTORY not in sys.path:
    sys.path.append(GM_DIRECTORY)
import gm_json
import jsondiff
import svn

USAGE_STRING = 'Usage: %s [options]'
HELP_STRING = '''

Generates a visual diff of all pending changes in the local SVN/git checkout.

This includes a list of all files that have been added, deleted, or modified
(as far as SVN/git knows about).  For any image modifications, pixel diffs will
be generated.

'''

IMAGE_FILENAME_RE = re.compile(gm_json.IMAGE_FILENAME_PATTERN)

TRUNK_PATH = os.path.join(os.path.dirname(__file__), os.pardir)

OPTION_DEST_DIR = '--dest-dir'
OPTION_PATH_TO_SKDIFF = '--path-to-skdiff'
OPTION_SOURCE_DIR = '--source-dir'

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

    extension = ''
    if os.name is 'nt':
        extension = '.exe'
        
    possible_paths = [os.path.join(trunk_path, 'out', 'Release',
                                    'skdiff' + extension),
                      os.path.join(trunk_path, 'out', 'Debug',
                                   'skdiff' + extension)]
    for try_path in possible_paths:
        if os.path.isfile(try_path):
            return try_path
    raise Exception('cannot find skdiff in paths %s; maybe you need to '
                    'specify the %s option or build skdiff?' % (
                        possible_paths, OPTION_PATH_TO_SKDIFF))

def _DownloadUrlToFile(source_url, dest_path):
    """Download source_url, and save its contents to dest_path.
    Raises an exception if there were any problems."""
    try:
        reader = urllib2.urlopen(source_url)
        writer = open(dest_path, 'wb')
        writer.write(reader.read())
        writer.close()
    except BaseException as e:
        raise Exception(
            '%s: unable to download source_url %s to dest_path %s' % (
                e, source_url, dest_path))

def _CreateGSUrl(imagename, hash_type, hash_digest):
    """Return the HTTP URL we can use to download this particular version of
    the actually-generated GM image with this imagename.

    imagename: name of the test image, e.g. 'perlinnoise_msaa4.png'
    hash_type: string indicating the hash type used to generate hash_digest,
               e.g. gm_json.JSONKEY_HASHTYPE_BITMAP_64BITMD5
    hash_digest: the hash digest of the image to retrieve
    """
    return gm_json.CreateGmActualUrl(
        test_name=IMAGE_FILENAME_RE.match(imagename).group(1),
        hash_type=hash_type,
        hash_digest=hash_digest)

def _CallJsonDiff(old_json_path, new_json_path,
                  old_flattened_dir, new_flattened_dir,
                  filename_prefix):
    """Using jsondiff.py, write the images that differ between two GM
    expectations summary files (old and new) into old_flattened_dir and
    new_flattened_dir.

    filename_prefix: prefix to prepend to filenames of all images we write
        into the flattened directories
    """
    json_differ = jsondiff.GMDiffer()
    diff_dict = json_differ.GenerateDiffDict(oldfile=old_json_path,
                                             newfile=new_json_path)
    print 'Downloading %d before-and-after image pairs...' % len(diff_dict)
    for (imagename, results) in diff_dict.iteritems():
        # TODO(epoger): Currently, this assumes that all images have been
        # checksummed using gm_json.JSONKEY_HASHTYPE_BITMAP_64BITMD5

        old_checksum = results['old']
        if old_checksum:
            old_image_url = _CreateGSUrl(
                imagename=imagename,
                hash_type=gm_json.JSONKEY_HASHTYPE_BITMAP_64BITMD5,
                hash_digest=old_checksum)
            _DownloadUrlToFile(
                source_url=old_image_url,
                dest_path=os.path.join(old_flattened_dir,
                                       filename_prefix + imagename))

        new_checksum = results['new']
        if new_checksum:
            new_image_url = _CreateGSUrl(
                imagename=imagename,
                hash_type=gm_json.JSONKEY_HASHTYPE_BITMAP_64BITMD5,
                hash_digest=new_checksum)
            _DownloadUrlToFile(
                source_url=new_image_url,
                dest_path=os.path.join(new_flattened_dir,
                                       filename_prefix + imagename))

def _RunCommand(args):
    """Run a command (from self._directory) and return stdout as a single
    string.

    @param args a list of arguments
    """
    proc = subprocess.Popen(args,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    (stdout, stderr) = proc.communicate()
    if proc.returncode is not 0:
        raise Exception('command "%s" failed: %s' % (args, stderr))
    return stdout

def _GitGetModifiedFiles():
    """Returns a list of locally modified files within the current working dir.

    TODO(epoger): Move this into a git utility package?
    """
    return _RunCommand(['git', 'ls-files', '-m']).splitlines()

def _GitExportBaseVersionOfFile(file_within_repo, dest_path):
    """Retrieves a copy of the base version of a file within the repository.

    @param file_within_repo path to the file within the repo whose base
           version you wish to obtain
    @param dest_path destination to which to write the base content

    TODO(epoger): Move this into a git utility package?
    """
    # TODO(epoger): Replace use of "git show" command with lower-level git
    # commands?  senorblanco points out that "git show" is a "porcelain"
    # command, intended for human use, as opposed to the "plumbing" commands
    # generally more suitable for scripting.  (See
    # http://git-scm.com/book/en/Git-Internals-Plumbing-and-Porcelain )
    #
    # For now, though, "git show" is the most straightforward implementation
    # I could come up with.  I tried using "git cat-file", but I had trouble
    # getting it to work as desired.
    # Note that git expects / rather than \ as a path separator even on
    # windows.
    args = ['git', 'show', posixpath.join('HEAD:.', file_within_repo)]
    with open(dest_path, 'wb') as outfile:
        proc = subprocess.Popen(args, stdout=outfile)
        proc.communicate()
        if proc.returncode is not 0:
            raise Exception('command "%s" failed' % args)

def SvnDiff(path_to_skdiff, dest_dir, source_dir):
    """Generates a visual diff of all pending changes in source_dir.

    @param path_to_skdiff
    @param dest_dir existing directory within which to write results
    @param source_dir
    """
    # Validate parameters, filling in default values if necessary and possible.
    path_to_skdiff = os.path.abspath(FindPathToSkDiff(path_to_skdiff))
    if not dest_dir:
        dest_dir = tempfile.mkdtemp()
    dest_dir = os.path.abspath(dest_dir)

    os.chdir(source_dir)
    svn_repo = svn.Svn('.')
    using_svn = True
    try:
      svn_repo.GetInfo()
    except:
      using_svn = False

    # Prepare temporary directories.
    modified_flattened_dir = os.path.join(dest_dir, 'modified_flattened')
    original_flattened_dir = os.path.join(dest_dir, 'original_flattened')
    diff_dir = os.path.join(dest_dir, 'diffs')
    for dir in [modified_flattened_dir, original_flattened_dir, diff_dir] :
        shutil.rmtree(dir, ignore_errors=True)
        os.mkdir(dir)

    # Get a list of all locally modified (including added/deleted) files,
    # descending subdirectories.
    if using_svn:
        modified_file_paths = svn_repo.GetFilesWithStatus(
            svn.STATUS_ADDED | svn.STATUS_DELETED | svn.STATUS_MODIFIED)
    else:
        modified_file_paths = _GitGetModifiedFiles()

    # For each modified file:
    # 1. copy its current contents into modified_flattened_dir
    # 2. copy its original contents into original_flattened_dir
    for modified_file_path in modified_file_paths:
        if modified_file_path.endswith('.json'):
            # Special handling for JSON files, in the hopes that they
            # contain GM result summaries.
            original_file = tempfile.NamedTemporaryFile(delete = False)
            original_file.close()
            if using_svn:
                svn_repo.ExportBaseVersionOfFile(
                    modified_file_path, original_file.name)
            else:
                _GitExportBaseVersionOfFile(
                    modified_file_path, original_file.name)
            modified_dir = os.path.dirname(modified_file_path)
            platform_prefix = (re.sub(re.escape(os.sep), '__',
                                      os.path.splitdrive(modified_dir)[1])
                              + '__')
            _CallJsonDiff(old_json_path=original_file.name,
                          new_json_path=modified_file_path,
                          old_flattened_dir=original_flattened_dir,
                          new_flattened_dir=modified_flattened_dir,
                          filename_prefix=platform_prefix)
            os.remove(original_file.name)
        else:
            dest_filename = re.sub(re.escape(os.sep), '__', modified_file_path)
            # If the file had STATUS_DELETED, it won't exist anymore...
            if os.path.isfile(modified_file_path):
                shutil.copyfile(modified_file_path,
                                os.path.join(modified_flattened_dir,
                                             dest_filename))
            if using_svn:
                svn_repo.ExportBaseVersionOfFile(
                    modified_file_path,
                    os.path.join(original_flattened_dir, dest_filename))
            else:
                _GitExportBaseVersionOfFile(
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
    SvnDiff(path_to_skdiff=options.path_to_skdiff, dest_dir=options.dest_dir,
            source_dir=options.source_dir)

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
    parser.add_option(OPTION_SOURCE_DIR,
                      action='store', type='string',
                      default=os.path.join('expectations', 'gm'),
                      help='root directory within which to compare all ' +
                      'files; defaults to "%default"')
    (options, args) = parser.parse_args()
    Main(options, args)
