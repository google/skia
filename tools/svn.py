'''
Copyright 2011 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

import fnmatch
import os
import re
import subprocess

PROPERTY_MIMETYPE = 'svn:mime-type'

class Svn:

    def __init__(self, directory):
        """Set up to manipulate SVN control within the given directory.

        @param directory
        """
        self._directory = directory

    def _RunCommand(self, args):
        """Run a command (from self._directory) and return stdout as a single
        string.

        @param args a list of arguments
        """
        print 'RunCommand: %s' % args
        proc = subprocess.Popen(args, cwd=self._directory,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (stdout, stderr) = proc.communicate()
        if proc.returncode is not 0:
            raise Exception('command "%s" failed in dir "%s": %s' %
                            (args, self._directory, stderr))
        return stdout

    def Checkout(self, url, path):
        """Check out a working copy from a repository.
        Returns stdout as a single string.

        @param url URL from which to check out the working copy
        @param path path (within self._directory) where the local copy will be
        written
        """
        return self._RunCommand(['svn', 'checkout', url, path])

    def GetNewFiles(self):
        """Return a list of files which are in this directory but NOT under
        SVN control.
        """
        stdout = self._RunCommand(['svn', 'status'])
        new_regex = re.compile('^\?.....\s+(.+)$', re.MULTILINE)
        files = new_regex.findall(stdout)
        return files

    def GetNewAndModifiedFiles(self):
        """Return a list of files in this dir which are newly added or modified,
        including those that are not (yet) under SVN control.
        """
        stdout = self._RunCommand(['svn', 'status'])
        new_regex = re.compile('^[AM\?].....\s+(.+)$', re.MULTILINE)
        files = new_regex.findall(stdout)
        return files

    def GetModifiedFiles(self):
        """Return a list of files in this dir which are under SVN control, and
        have been modified.
        """
        stdout = self._RunCommand(['svn', 'status'])
        new_regex = re.compile('^M.....\s+(.+)$', re.MULTILINE)
        files = new_regex.findall(stdout)
        return files

    def AddFiles(self, filenames):
        """Adds these files to SVN control.

        @param filenames files to add to SVN control
        """
        self._RunCommand(['svn', 'add'] + filenames)

    def SetProperty(self, filenames, property_name, property_value):
        """Sets a svn property for these files.

        @param filenames files to set property on
        @param property_name property_name to set for each file
        @param property_value what to set the property_name to
        """
        if filenames:
            self._RunCommand(
                ['svn', 'propset', property_name, property_value] + filenames)

    def SetPropertyByFilenamePattern(self, filename_pattern,
                                     property_name, property_value):
        """Sets a svn property for all files matching filename_pattern.

        @param filename_pattern set the property for all files whose names match
               this Unix-style filename pattern (e.g., '*.jpg')
        @param property_name property_name to set for each file
        @param property_value what to set the property_name to
        """
        all_files = os.listdir(self._directory)
        matching_files = sorted(fnmatch.filter(all_files, filename_pattern))
        self.SetProperty(matching_files, property_name, property_value)

    def ExportBaseVersionOfFile(self, file_within_repo, dest_path):
        """Retrieves a copy of the base version (what you would get if you ran
        'svn revert') of a file within the repository.

        @param file_within_repo path to the file within the repo whose base
               version you wish to obtain
        @param dest_path destination to which to write the base content
        """
        self._RunCommand(['svn', 'export', '--revision', 'BASE',
                          file_within_repo, dest_path])
