#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Module that sanitizes source files with specified modifiers."""


import commands
import os
import sys


_FILE_EXTENSIONS_TO_SANITIZE = ['cpp', 'h', 'c']

_SUBDIRS_TO_IGNORE = ['.git', '.svn', 'third_party']


def SanitizeFilesWithModifiers(directory, file_modifiers, line_modifiers):
  """Sanitizes source files with the specified file and line modifiers.

  Args:
    directory: string - The directory which will be recursively traversed to
        find source files to apply modifiers to.
    file_modifiers: list - file-modification methods which should be applied to
        the complete file content (Eg: EOFOneAndOnlyOneNewlineAdder).
    line_modifiers: list - line-modification methods which should be applied to
        lines in a file (Eg: TabReplacer).
  """
  for item in os.listdir(directory):

    full_item_path = os.path.join(directory, item)

    if os.path.isfile(full_item_path):  # Item is a file.

      # Only sanitize files with extensions we care about.
      if (len(full_item_path.split('.')) > 1 and
          full_item_path.split('.')[-1] in _FILE_EXTENSIONS_TO_SANITIZE):
        f = file(full_item_path)
        try:
          lines = f.readlines()
        finally:
          f.close()

        new_lines = []  # Collect changed lines here.
        line_number = 0  # Keeps track of line numbers in the source file.
        write_to_file = False  # File is written to only if this flag is set.

        # Run the line modifiers for each line in this file.
        for line in lines:
          original_line = line
          line_number += 1

          for modifier in line_modifiers:
            line = modifier(line, full_item_path, line_number)
            if original_line != line:
              write_to_file = True
          new_lines.append(line)

        # Run the file modifiers.
        old_content = ''.join(lines)
        new_content = ''.join(new_lines)
        for modifier in file_modifiers:
          new_content = modifier(new_content, full_item_path)
        if new_content != old_content:
          write_to_file = True

        # Write modifications to the file.
        if write_to_file:
          f = file(full_item_path, 'w')
          try:
            f.write(new_content)
          finally:
            f.close()
          print 'Made changes to %s' % full_item_path

    elif item not in _SUBDIRS_TO_IGNORE:
      # Item is a directory recursively call the method.
      SanitizeFilesWithModifiers(full_item_path, file_modifiers, line_modifiers)


############## Line Modification methods ##############


def TrailingWhitespaceRemover(line, file_path, line_number):
  """Strips out trailing whitespaces from the specified line."""
  stripped_line = line.rstrip() + '\n'
  if line != stripped_line:
    print 'Removing trailing whitespace in %s:%s' % (file_path, line_number)
  return stripped_line


def CrlfReplacer(line, file_path, line_number):
  """Replaces CRLF with LF."""
  if '\r\n' in line:
    print 'Replacing CRLF with LF in %s:%s' % (file_path, line_number)
  return line.replace('\r\n', '\n')


def TabReplacer(line, file_path, line_number):
  """Replaces Tabs with 4 whitespaces."""
  if '\t' in line:
    print 'Replacing Tab with whitespace in %s:%s' % (file_path, line_number)
  return line.replace('\t', '    ')


############## File Modification methods ##############


def CopywriteChecker(file_content, unused_file_path):
  """Ensures that the copywrite information is correct."""
  # TODO(rmistry): Figure out the legal implications of changing old copyright
  # headers.
  return file_content


def EOFOneAndOnlyOneNewlineAdder(file_content, file_path):
  """Adds one and only one LF at the end of the file."""
  if file_content and (file_content[-1] != '\n' or file_content[-2:-1] == '\n'):
    file_content = file_content.rstrip()
    file_content += '\n'
    print 'Added exactly one newline to %s' % file_path
  return file_content


def SvnEOLChecker(file_content, file_path):
  """Sets svn:eol-style property to LF."""
  output = commands.getoutput(
      'svn propget svn:eol-style %s' % file_path)
  if output != 'LF':
    print 'Setting svn:eol-style property to LF in %s' % file_path
    os.system('svn ps svn:eol-style LF %s' % file_path)
  return file_content


#######################################################


if '__main__' == __name__:
  sys.exit(SanitizeFilesWithModifiers(
      os.getcwd(),
      file_modifiers=[
          CopywriteChecker,
          EOFOneAndOnlyOneNewlineAdder,
          SvnEOLChecker,
      ],
      line_modifiers=[
          CrlfReplacer,
          TabReplacer,
          TrailingWhitespaceRemover,
      ],
  ))
