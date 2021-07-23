'''
Copyright 2011 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

'''
Updates all copyright headers within our code:
- For files that already have a copyright header, the header is modified
  while keeping the year and holder intact.
- For files that don't have a copyright header, we add one with the current
  year and default holder.

@author: epoger@google.com
'''


from __future__ import print_function
import os
import sys

import fileparser


# Only modify copyright stanzas if the copyright holder is one of these.
ALLOWED_COPYRIGHT_HOLDERS = [
    'Google Inc.',
    'Skia',
    'The Android Open Source Project',
]

def Main(root_directory):
    """Run everything.

    @param root_directory root directory within which to modify all files
    """
    filepaths = GetAllFilepaths(root_directory)
    for filepath in filepaths:
        parser = fileparser.CreateParser(filepath)
        if not parser:
            ReportWarning('cannot find a parser for file %s, skipping...' %
                          filepath)
            continue
        old_file_contents = ReadFileIntoString(filepath)
        comment_blocks = parser.FindAllCommentBlocks(old_file_contents)
        if not comment_blocks:
            ReportWarning('cannot find any comment blocks in file %s' %
                          filepath)
        old_copyright_block = parser.FindCopyrightBlock(comment_blocks)
        if not old_copyright_block:
            ReportWarning('cannot find copyright block in file %s' % filepath)
        (year, holder) = parser.GetCopyrightBlockAttributes(old_copyright_block)
        if holder and not ConfirmAllowedCopyrightHolder(holder):
            ReportWarning(
                'unrecognized copyright holder "%s" in file %s, skipping...' % (
                    holder, filepath))
            continue
        new_copyright_block = parser.CreateCopyrightBlock(year, holder)
        if old_copyright_block:
            new_file_contents = old_file_contents.replace(
                old_copyright_block, new_copyright_block, 1)
        else:
            new_file_contents = new_copyright_block + old_file_contents
        WriteStringToFile(new_file_contents, filepath)


def GetAllFilepaths(root_directory):
    """Return a list of all files (absolute path for each one) within a tree.

    @param root_directory root directory within which to find all files
    """
    path_list = []
    for dirpath, _, filenames in os.walk(root_directory):
        for filename in filenames:
            path_list.append(os.path.abspath(os.path.join(dirpath, filename)))
    return path_list


def ReportWarning(text):
    """Report a warning, but continue.
    """
    print('warning: %s' % text)


def ReportError(text):
    """Report an error and raise an exception.
    """
    raise IOError(text)


def ReadFileIntoString(filepath):
    """Returns the full contents of this file as a string.
    """
    with open(filepath, 'r') as file_handle:
        contents = file_handle.read()
    return contents


def WriteStringToFile(string, filepath):
    """Writes this string out to filepath, replacing the file if it already
    exists.
    """
    with open(filepath, 'w') as file_handle:
        file_handle.write(string)


def ConfirmAllowedCopyrightHolder(holder):
    """Returns True if this is one of our allowed copyright holders.

    @param holder copyright holder as a string
    """
    return holder in ALLOWED_COPYRIGHT_HOLDERS


Main(sys.argv[1])
