# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Module to host the VerboseSubprocess, ChangeDir, and ReSearch classes.
"""

import os
import re
import subprocess


def print_subprocess_args(prefix, *args, **kwargs):
    """Print out args in a human-readable manner."""
    def quote_and_escape(string):
        """Quote and escape a string if necessary."""
        if ' ' in string or '\n' in string:
            string = '"%s"' % string.replace('"', '\\"')
        return string
    if 'cwd' in kwargs:
        print '%scd %s' % (prefix, kwargs['cwd'])
    print prefix + ' '.join(quote_and_escape(arg) for arg in args[0])
    if 'cwd' in kwargs:
        print '%scd -' % prefix


class VerboseSubprocess(object):
    """Call subprocess methods, but print out command before executing.

    Attributes:
        verbose: (boolean) should we print out the command or not.  If
                 not, this is the same as calling the subprocess method
        quiet: (boolean) suppress stdout on check_call and call.
        prefix: (string) When verbose, what to print before each command.
    """

    def __init__(self, verbose):
        self.verbose = verbose
        self.quiet = not verbose
        self.prefix = '~~$ '

    def check_call(self, *args, **kwargs):
        """Wrapper for subprocess.check_call().

        Args:
            *args: to be passed to subprocess.check_call()
            **kwargs: to be passed to subprocess.check_call()
        Returns:
            Whatever subprocess.check_call() returns.
        Raises:
            OSError or subprocess.CalledProcessError: raised by check_call.
        """
        if self.verbose:
            print_subprocess_args(self.prefix, *args, **kwargs)
        if self.quiet:
            with open(os.devnull, 'w') as devnull:
                return subprocess.check_call(*args, stdout=devnull, **kwargs)
        else:
            return subprocess.check_call(*args, **kwargs)

    def call(self, *args, **kwargs):
        """Wrapper for subprocess.check().

        Args:
            *args: to be passed to subprocess.check_call()
            **kwargs: to be passed to subprocess.check_call()
        Returns:
            Whatever subprocess.call() returns.
        Raises:
            OSError or subprocess.CalledProcessError: raised by call.
        """
        if self.verbose:
            print_subprocess_args(self.prefix, *args, **kwargs)
        if self.quiet:
            with open(os.devnull, 'w') as devnull:
                return subprocess.call(*args, stdout=devnull, **kwargs)
        else:
            return subprocess.call(*args, **kwargs)

    def check_output(self, *args, **kwargs):
        """Wrapper for subprocess.check_output().

        Args:
            *args: to be passed to subprocess.check_output()
            **kwargs: to be passed to subprocess.check_output()
        Returns:
            Whatever subprocess.check_output() returns.
        Raises:
            OSError or subprocess.CalledProcessError: raised by check_output.
        """
        if self.verbose:
            print_subprocess_args(self.prefix, *args, **kwargs)
        return subprocess.check_output(*args, **kwargs)

    def strip_output(self, *args, **kwargs):
        """Wrap subprocess.check_output and str.strip().

        Pass the given arguments into subprocess.check_output() and return
        the results, after stripping any excess whitespace.

        Args:
            *args: to be passed to subprocess.check_output()
            **kwargs: to be passed to subprocess.check_output()

        Returns:
            The output of the process as a string without leading or
            trailing whitespace.
        Raises:
            OSError or subprocess.CalledProcessError: raised by check_output.
        """
        if self.verbose:
            print_subprocess_args(self.prefix, *args, **kwargs)
        return str(subprocess.check_output(*args, **kwargs)).strip()

    def popen(self, *args, **kwargs):
        """Wrapper for subprocess.Popen().

        Args:
            *args: to be passed to subprocess.Popen()
            **kwargs: to be passed to subprocess.Popen()
        Returns:
            The output of subprocess.Popen()
        Raises:
            OSError or subprocess.CalledProcessError: raised by Popen.
        """
        if self.verbose:
            print_subprocess_args(self.prefix, *args, **kwargs)
        return subprocess.Popen(*args, **kwargs)


class ChangeDir(object):
    """Use with a with-statement to temporarily change directories."""
    # pylint: disable=I0011,R0903

    def __init__(self, directory, verbose=False):
        self._directory = directory
        self._verbose = verbose

    def __enter__(self):
        if self._directory != os.curdir:
            if self._verbose:
                print '~~$ cd %s' % self._directory
            cwd = os.getcwd()
            os.chdir(self._directory)
            self._directory = cwd

    def __exit__(self, etype, value, traceback):
        if self._directory != os.curdir:
            if self._verbose:
                print '~~$ cd %s' % self._directory
            os.chdir(self._directory)


class ReSearch(object):
    """A collection of static methods for regexing things."""

    @staticmethod
    def search_within_stream(input_stream, pattern, default=None):
        """Search for regular expression in a file-like object.

        Opens a file for reading and searches line by line for a match to
        the regex and returns the parenthesized group named return for the
        first match.  Does not search across newlines.

        For example:
            pattern = '^root(:[^:]*){4}:(?P<return>[^:]*)'
            with open('/etc/passwd', 'r') as stream:
                return search_within_file(stream, pattern)
        should return root's home directory (/root on my system).

        Args:
            input_stream: file-like object to be read
            pattern: (string) to be passed to re.compile
            default: what to return if no match

        Returns:
            A string or whatever default is
        """
        pattern_object = re.compile(pattern)
        for line in input_stream:
            match = pattern_object.search(line)
            if match:
                return match.group('return')
        return default

    @staticmethod
    def search_within_string(input_string, pattern, default=None):
        """Search for regular expression in a string.

        Args:
            input_string: (string) to be searched
            pattern: (string) to be passed to re.compile
            default: what to return if no match

        Returns:
            A string or whatever default is
        """
        match = re.search(pattern, input_string)
        return match.group('return') if match else default

    @staticmethod
    def search_within_output(verbose, pattern, default, *args, **kwargs):
        """Search for regular expression in a process output.

        Does not search across newlines.

        Args:
            verbose: (boolean) shoule we call print_subprocess_args?
            pattern: (string) to be passed to re.compile
            default: what to return if no match
            *args: to be passed to subprocess.Popen()
            **kwargs: to be passed to subprocess.Popen()

        Returns:
            A string or whatever default is
        """
        if verbose:
            print_subprocess_args('~~$ ', *args, **kwargs)
        proc = subprocess.Popen(*args, stdout=subprocess.PIPE, **kwargs)
        return ReSearch.search_within_stream(proc.stdout, pattern, default)


