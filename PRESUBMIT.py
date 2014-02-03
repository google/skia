# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Top-level presubmit script for Skia.

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into gcl.
"""

import fnmatch
import os
import re
import sys
import traceback


REVERT_CL_SUBJECT_PREFIX = 'Revert '

SKIA_TREE_STATUS_URL = 'http://skia-tree-status.appspot.com'

PUBLIC_API_OWNERS = (
    'reed@chromium.org',
    'reed@google.com',
    'bsalomon@chromium.org',
    'bsalomon@google.com',
)

AUTHORS_FILE_NAME = 'AUTHORS'


def _CheckChangeHasEol(input_api, output_api, source_file_filter=None):
  """Checks that files end with atleast one \n (LF)."""
  eof_files = []
  for f in input_api.AffectedSourceFiles(source_file_filter):
    contents = input_api.ReadFile(f, 'rb')
    # Check that the file ends in atleast one newline character.
    if len(contents) > 1 and contents[-1:] != '\n':
      eof_files.append(f.LocalPath())

  if eof_files:
    return [output_api.PresubmitPromptWarning(
      'These files should end in a newline character:',
      items=eof_files)]
  return []


def _CommonChecks(input_api, output_api):
  """Presubmit checks common to upload and commit."""
  results = []
  sources = lambda x: (x.LocalPath().endswith('.h') or
                       x.LocalPath().endswith('.gypi') or
                       x.LocalPath().endswith('.gyp') or
                       x.LocalPath().endswith('.py') or
                       x.LocalPath().endswith('.sh') or
                       x.LocalPath().endswith('.cpp'))
  results.extend(
      _CheckChangeHasEol(
          input_api, output_api, source_file_filter=sources))
  return results


def CheckChangeOnUpload(input_api, output_api):
  """Presubmit checks for the change on upload.

  The following are the presubmit checks:
  * Check change has one and only one EOL.
  """
  results = []
  results.extend(_CommonChecks(input_api, output_api))
  return results


def _CheckTreeStatus(input_api, output_api, json_url):
  """Check whether to allow commit.

  Args:
    input_api: input related apis.
    output_api: output related apis.
    json_url: url to download json style status.
  """
  tree_status_results = input_api.canned_checks.CheckTreeIsOpen(
      input_api, output_api, json_url=json_url)
  if not tree_status_results:
    # Check for caution state only if tree is not closed.
    connection = input_api.urllib2.urlopen(json_url)
    status = input_api.json.loads(connection.read())
    connection.close()
    if ('caution' in status['message'].lower() and
        os.isatty(sys.stdout.fileno())):
      # Display a prompt only if we are in an interactive shell. Without this
      # check the commit queue behaves incorrectly because it considers
      # prompts to be failures.
      short_text = 'Tree state is: ' + status['general_state']
      long_text = status['message'] + '\n' + json_url
      tree_status_results.append(
          output_api.PresubmitPromptWarning(
              message=short_text, long_text=long_text))
  else:
    # Tree status is closed. Put in message about contacting sheriff.
    connection = input_api.urllib2.urlopen(
        SKIA_TREE_STATUS_URL + '/current-sheriff')
    sheriff_details = input_api.json.loads(connection.read())
    if sheriff_details:
      tree_status_results[0]._message += (
          '\n\nPlease contact the current Skia sheriff (%s) if you are trying '
          'to submit a build fix\nand do not know how to submit because the '
          'tree is closed') % sheriff_details['username']
  return tree_status_results


def _CheckOwnerIsInAuthorsFile(input_api, output_api):
  results = []
  issue = input_api.change.issue
  if issue and input_api.rietveld:
    issue_properties = input_api.rietveld.get_issue_properties( 
        issue=int(issue), messages=False)
    owner_email = issue_properties['owner_email']

    try:
      authors_content = ''
      for line in open(AUTHORS_FILE_NAME):
        if not line.startswith('#'):
          authors_content += line
      email_fnmatches = re.findall('<(.*)>', authors_content)
      for email_fnmatch in email_fnmatches:
        if fnmatch.fnmatch(owner_email, email_fnmatch):
          # Found a match, the user is in the AUTHORS file break out of the loop
          break
      else:
        # TODO(rmistry): Remove the below CLA messaging once a CLA checker has
        # been added to the CQ.
        results.append(
          output_api.PresubmitError(
            'The email %s is not in Skia\'s AUTHORS file.\n'
            'Issue owner, this CL must include an addition to the Skia AUTHORS '
            'file.\n'
            'Googler reviewers, please check that the AUTHORS entry '
            'corresponds to an email address in http://goto/cla-signers. If it '
            'does not then ask the issue owner to sign the CLA at '
            'https://developers.google.com/open-source/cla/individual '
            '(individual) or '
            'https://developers.google.com/open-source/cla/corporate '
            '(corporate).'
            % owner_email)) 
    except IOError:
      # Do not fail if authors file cannot be found.
      traceback.print_exc()
      input_api.logging.error('AUTHORS file not found!')

  return results


def _CheckLGTMsForPublicAPI(input_api, output_api):
  """Check LGTMs for public API changes.

  For public API files make sure there is an LGTM from the list of owners in
  PUBLIC_API_OWNERS.
  """
  results = []
  requires_owner_check = False
  for affected_svn_file in input_api.AffectedFiles():
    affected_file_path = affected_svn_file.AbsoluteLocalPath()
    file_path, file_ext = os.path.splitext(affected_file_path)
    # We only care about files that end in .h and are under the include dir.
    if file_ext == '.h' and 'include' in file_path.split(os.path.sep):
      requires_owner_check = True

  if not requires_owner_check:
    return results

  lgtm_from_owner = False
  issue = input_api.change.issue
  if issue and input_api.rietveld:
    issue_properties = input_api.rietveld.get_issue_properties(
        issue=int(issue), messages=True)
    if re.match(REVERT_CL_SUBJECT_PREFIX, issue_properties['subject'], re.I):
      # It is a revert CL, ignore the public api owners check.
      return results
    if issue_properties['owner_email'] in PUBLIC_API_OWNERS:
      # An owner created the CL that is an automatic LGTM.
      lgtm_from_owner = True

    messages = issue_properties.get('messages')
    if messages:
      for message in messages:
        if (message['sender'] in PUBLIC_API_OWNERS and
            'lgtm' in message['text'].lower()):
          # Found an lgtm in a message from an owner.
          lgtm_from_owner = True
          break;

  if not lgtm_from_owner:
    results.append(
        output_api.PresubmitError(
            'Since the CL is editing public API, you must have an LGTM from '
            'one of: %s' % str(PUBLIC_API_OWNERS)))
  return results


def CheckChangeOnCommit(input_api, output_api):
  """Presubmit checks for the change on commit.

  The following are the presubmit checks:
  * Check change has one and only one EOL.
  * Ensures that the Skia tree is open in
    http://skia-tree-status.appspot.com/. Shows a warning if it is in 'Caution'
    state and an error if it is in 'Closed' state.
  """
  results = []
  results.extend(_CommonChecks(input_api, output_api))
  results.extend(
      _CheckTreeStatus(input_api, output_api, json_url=(
          SKIA_TREE_STATUS_URL + '/banner-status?format=json')))
  results.extend(_CheckLGTMsForPublicAPI(input_api, output_api))
  results.extend(_CheckOwnerIsInAuthorsFile(input_api, output_api))
  return results
