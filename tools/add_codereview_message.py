#!/usr/bin/python2

# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Add message to codereview issue.

This script takes a codereview URL or a codereview issue number as its
argument and a (possibly multi-line) message on stdin.  It then calls
`git cl upload` to append the message to the given codereview issue.

Usage:
  echo MESSAGE | %prog -c CHECKOUT_PATH CODEREVIEW_ISSUE
or:
  cd /path/to/git/checkout
  %prog CODEREVIEW_ISSUE <<EOF
  MESSAGE
  EOF
or:
  %prog --help
"""

import optparse
import os
import sys

import git_utils
import misc_utils


DEFAULT_REVIEWERS = ','.join([
    'rmistry@google.com',
    'reed@google.com',
    'bsalomon@google.com',
    'robertphillips@google.com',
    ])


DEFAULT_CC_LIST = ','.join([
    'skia-team@google.com',
    ])


def add_codereview_message(codereview_url, message, checkout_path,
                           skip_cl_upload, verbose, reviewers, cclist):
    """Add a message to a given codereview.

    Args:
        codereview_url: (string) we will extract the issue number from
            this url, or this could simply be the issue number.
        message: (string) will be passed to `git cl upload -m $MESSAGE`
        checkout_path: (string) location of the git
            repository checkout to be used.
        skip_cl_upload: (boolean) if true, don't actually
            add the message and keep the temporary branch around.
        verbose: (boolean) print out details useful for debugging.
        reviewers: (string) comma-separated list of reviewers
        cclist: (string) comma-separated list of addresses to be
            carbon-copied
    """
    # pylint: disable=I0011,R0913
    git = git_utils.git_executable()
    issue = codereview_url.strip('/').split('/')[-1]
    vsp = misc_utils.VerboseSubprocess(verbose)
    if skip_cl_upload:
        branch_name = 'issue_%s' % issue
    else:
        branch_name = None
    upstream = 'origin/master'

    with misc_utils.ChangeDir(checkout_path, verbose):
        vsp.check_call([git, 'fetch', '-q', 'origin'])

        with git_utils.ChangeGitBranch(branch_name, upstream, verbose):
            vsp.check_call([git, 'cl', 'patch', issue])

            git_upload = [
                git, 'cl', 'upload', '-t', 'bot report', '-m', message]
            if cclist:
                git_upload.append('--cc=' + cclist)
            if reviewers:
                git_upload.append('--reviewers=' + reviewers)

            if skip_cl_upload:
                branch_name = git_utils.git_branch_name(verbose)
                space = '    '
                print 'You should call:'
                misc_utils.print_subprocess_args(space, ['cd', os.getcwd()])
                misc_utils.print_subprocess_args(
                    space, [git, 'checkout', branch_name])
                misc_utils.print_subprocess_args(space, git_upload)
            else:
                vsp.check_call(git_upload)
                print vsp.check_output([git, 'cl', 'issue'])


def main(argv):
    """main function; see module-level docstring and GetOptionParser help.

    Args:
        argv: sys.argv[1:]-type argument list.
    """
    option_parser = optparse.OptionParser(usage=__doc__)
    option_parser.add_option(
        '-c', '--checkout_path',
        default=os.curdir,
        help='Path to the Git repository checkout,'
        ' defaults to current working directory.')
    option_parser.add_option(
        '', '--skip_cl_upload', action='store_true', default=False,
        help='Skip the cl upload step; useful for testing.')
    option_parser.add_option(
        '', '--verbose', action='store_true', dest='verbose', default=False,
        help='Do not suppress the output from `git cl`.',)
    option_parser.add_option(
        '', '--git_path', default='git',
        help='Git executable, defaults to "git".',)
    option_parser.add_option(
        '', '--reviewers', default=DEFAULT_REVIEWERS,
        help=('Comma-separated list of reviewers.  Default is "%s".'
              % DEFAULT_REVIEWERS))
    option_parser.add_option(
        '', '--cc', default=DEFAULT_CC_LIST,
        help=('Comma-separated list of addresses to be carbon-copied.'
              '  Default is "%s".' %  DEFAULT_CC_LIST))

    options, arguments = option_parser.parse_args(argv)

    if not options.checkout_path:
        option_parser.error('Must specify checkout_path.')
    if not git_utils.git_executable():
        option_parser.error('Invalid git executable.')
    if len(arguments) > 1:
        option_parser.error('Extra arguments.')
    if len(arguments) != 1:
        option_parser.error('Missing Codereview URL.')

    message = sys.stdin.read()
    add_codereview_message(arguments[0], message, options.checkout_path,
                           options.skip_cl_upload, options.verbose,
                           options.reviewers, options.cc)


if __name__ == '__main__':
    main(sys.argv[1:])

