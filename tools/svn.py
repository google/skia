'''
Copyright 2011 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

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
        proc = subprocess.Popen(args, cwd=self._directory,
                                stdout=subprocess.PIPE)
        stdout = proc.communicate()[0]
        returncode = proc.returncode
        if returncode is not 0:
            raise Exception('command "%s" failed in dir "%s": returncode=%s' %
                            (args, self._directory, returncode))
        return stdout

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

    def AddFiles(self, filenames):
        """Adds these files to SVN control.

        @param filenames files to add to SVN control
        """
        args = ['svn', 'add']
        args.extend(filenames)
        print '\n\nAddFiles: %s' % args
        print self._RunCommand(args)

    def SetProperty(self, filenames, property_name, property_value):
        """Sets a svn property for these files.

        @param filenames files to set property on
        @param property_name property_name to set for each file
        @param property_value what to set the property_name to
        """
        args = ['svn', 'propset', property_name, property_value]
        args.extend(filenames)
        print '\n\nSetProperty: %s' % args
        print self._RunCommand(args)
