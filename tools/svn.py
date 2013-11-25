'''
Copyright 2011 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

import fnmatch
import os
import re
import subprocess
import threading

PROPERTY_MIMETYPE = 'svn:mime-type'

# Status types for GetFilesWithStatus()
STATUS_ADDED                 = 0x01
STATUS_DELETED               = 0x02
STATUS_MODIFIED              = 0x04
STATUS_NOT_UNDER_SVN_CONTROL = 0x08


if os.name == 'nt':
  SVN = 'svn.bat'
else:
  SVN = 'svn'


def Cat(svn_url):
    """Returns the contents of the file at the given svn_url. 

    @param svn_url URL of the file to read
    """
    proc = subprocess.Popen([SVN, 'cat', svn_url],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
    exitcode = proc.wait()
    if not exitcode == 0:
        raise Exception('Could not retrieve %s. Verify that the URL is valid '
                        'and check your connection.' % svn_url)
    return proc.communicate()[0]


class Svn:

    def __init__(self, directory):
        """Set up to manipulate SVN control within the given directory.

        The resulting object is thread-safe: access to all methods is
        synchronized (if one thread is currently executing any of its methods,
        all other threads must wait before executing any of its methods).

        @param directory
        """
        self._directory = directory
        # This must be a reentrant lock, so that it can be held by both
        # _RunCommand() and (some of) the methods that call it.
        self._rlock = threading.RLock()

    def _RunCommand(self, args):
        """Run a command (from self._directory) and return stdout as a single
        string.

        @param args a list of arguments
        """
        with self._rlock:
            print 'RunCommand: %s' % args
            proc = subprocess.Popen(args, cwd=self._directory,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
            (stdout, stderr) = proc.communicate()
            if proc.returncode is not 0:
              raise Exception('command "%s" failed in dir "%s": %s' %
                              (args, self._directory, stderr))
            return stdout

    def GetInfo(self):
        """Run "svn info" and return a dictionary containing its output.
        """
        output = self._RunCommand([SVN, 'info'])
        svn_info = {}
        for line in output.split('\n'):
          if ':' in line:
            (key, value) = line.split(':', 1)
            svn_info[key.strip()] = value.strip()
        return svn_info

    def Checkout(self, url, path):
        """Check out a working copy from a repository.
        Returns stdout as a single string.

        @param url URL from which to check out the working copy
        @param path path (within self._directory) where the local copy will be
        written
        """
        return self._RunCommand([SVN, 'checkout', url, path])

    def Update(self, path):
        """Update the working copy.
        Returns stdout as a single string.

        @param path path (within self._directory) within which to run
        "svn update"
        """
        return self._RunCommand([SVN, 'update', path])

    def ListSubdirs(self, url):
        """Returns a list of all subdirectories (not files) within a given SVN
        url.

        @param url remote directory to list subdirectories of
        """
        subdirs = []
        filenames = self._RunCommand([SVN, 'ls', url]).split('\n')
        for filename in filenames:
            if filename.endswith('/'):
                subdirs.append(filename.strip('/'))
        return subdirs

    def GetNewFiles(self):
        """Return a list of files which are in this directory but NOT under
        SVN control.
        """
        return self.GetFilesWithStatus(STATUS_NOT_UNDER_SVN_CONTROL)

    def GetNewAndModifiedFiles(self):
        """Return a list of files in this dir which are newly added or modified,
        including those that are not (yet) under SVN control.
        """
        return self.GetFilesWithStatus(
            STATUS_ADDED | STATUS_MODIFIED | STATUS_NOT_UNDER_SVN_CONTROL)

    def GetFilesWithStatus(self, status):
        """Return a list of files in this dir with the given SVN status.

        @param status bitfield combining one or more STATUS_xxx values
        """
        status_types_string = ''
        if status & STATUS_ADDED:
            status_types_string += 'A'
        if status & STATUS_DELETED:
            status_types_string += 'D'
        if status & STATUS_MODIFIED:
            status_types_string += 'M'
        if status & STATUS_NOT_UNDER_SVN_CONTROL:
            status_types_string += '\?'
        status_regex_string = '^[%s].....\s+(.+)$' % status_types_string
        stdout = self._RunCommand([SVN, 'status']).replace('\r', '')
        status_regex = re.compile(status_regex_string, re.MULTILINE)
        files = status_regex.findall(stdout)
        return files

    def AddFiles(self, filenames):
        """Adds these files to SVN control.

        @param filenames files to add to SVN control
        """
        self._RunCommand([SVN, 'add'] + filenames)

    def SetProperty(self, filenames, property_name, property_value):
        """Sets a svn property for these files.

        @param filenames files to set property on
        @param property_name property_name to set for each file
        @param property_value what to set the property_name to
        """
        if filenames:
            self._RunCommand(
                [SVN, 'propset', property_name, property_value] + filenames)

    def SetPropertyByFilenamePattern(self, filename_pattern,
                                     property_name, property_value):
        """Sets a svn property for all files matching filename_pattern.

        @param filename_pattern set the property for all files whose names match
               this Unix-style filename pattern (e.g., '*.jpg')
        @param property_name property_name to set for each file
        @param property_value what to set the property_name to
        """
        with self._rlock:
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
        self._RunCommand([SVN, 'export', '--revision', 'BASE', '--force',
                          file_within_repo, dest_path])
