# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Top-level presubmit script for Skia.

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into gcl.
"""

import csv
import fnmatch
import os
import re
import subprocess
import sys
import traceback


REVERT_CL_SUBJECT_PREFIX = 'Revert '

SKIA_TREE_STATUS_URL = 'http://skia-tree-status.appspot.com'

CQ_KEYWORDS_THAT_NEED_APPENDING = ('CQ_INCLUDE_TRYBOTS', 'CQ_EXTRA_TRYBOTS',
                                   'CQ_EXCLUDE_TRYBOTS', 'CQ_TRYBOTS')

# Please add the complete email address here (and not just 'xyz@' or 'xyz').
PUBLIC_API_OWNERS = (
    'reed@chromium.org',
    'reed@google.com',
    'bsalomon@chromium.org',
    'bsalomon@google.com',
    'djsollen@chromium.org',
    'djsollen@google.com',
)

AUTHORS_FILE_NAME = 'AUTHORS'

DOCS_PREVIEW_URL = 'https://skia.org/?cl='


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


def _PythonChecks(input_api, output_api):
  """Run checks on any modified Python files."""
  pylint_disabled_warnings = (
      'F0401',  # Unable to import.
      'E0611',  # No name in module.
      'W0232',  # Class has no __init__ method.
      'E1002',  # Use of super on an old style class.
      'W0403',  # Relative import used.
      'R0201',  # Method could be a function.
      'E1003',  # Using class name in super.
      'W0613',  # Unused argument.
  )
  # Run Pylint on only the modified python files. Unfortunately it still runs
  # Pylint on the whole file instead of just the modified lines.
  affected_python_files = []
  for affected_file in input_api.AffectedSourceFiles(None):
    affected_file_path = affected_file.LocalPath()
    if affected_file_path.endswith('.py'):
      affected_python_files.append(affected_file_path)
  return input_api.canned_checks.RunPylint(
      input_api, output_api,
      disabled_warnings=pylint_disabled_warnings,
      white_list=affected_python_files)


def _IfDefChecks(input_api, output_api):
  """Ensures if/ifdef are not before includes. See skbug/3362 for details."""
  comment_block_start_pattern = re.compile('^\s*\/\*.*$')
  comment_block_middle_pattern = re.compile('^\s+\*.*')
  comment_block_end_pattern = re.compile('^\s+\*\/.*$')
  single_line_comment_pattern = re.compile('^\s*//.*$')
  def is_comment(line):
    return (comment_block_start_pattern.match(line) or
            comment_block_middle_pattern.match(line) or
            comment_block_end_pattern.match(line) or
            single_line_comment_pattern.match(line))

  empty_line_pattern = re.compile('^\s*$')
  def is_empty_line(line):
    return empty_line_pattern.match(line)

  failing_files = []
  for affected_file in input_api.AffectedSourceFiles(None):
    affected_file_path = affected_file.LocalPath()
    if affected_file_path.endswith('.cpp') or affected_file_path.endswith('.h'):
      f = open(affected_file_path)
      for line in f.xreadlines():
        if is_comment(line) or is_empty_line(line):
          continue
        # The below will be the first real line after comments and newlines.
        if line.startswith('#if 0 '):
          pass
        elif line.startswith('#if ') or line.startswith('#ifdef '):
          failing_files.append(affected_file_path)
        break

  results = []
  if failing_files:
    results.append(
        output_api.PresubmitError(
            'The following files have #if or #ifdef before includes:\n%s\n\n'
            'See skbug.com/3362 for why this should be fixed.' %
                '\n'.join(failing_files)))
  return results


def _CopyrightChecks(input_api, output_api, source_file_filter=None):
  results = []
  year_pattern = r'\d{4}'
  year_range_pattern = r'%s(-%s)?' % (year_pattern, year_pattern)
  years_pattern = r'%s(,%s)*,?' % (year_range_pattern, year_range_pattern)
  copyright_pattern = (
      r'Copyright (\([cC]\) )?%s \w+' % years_pattern)

  for affected_file in input_api.AffectedSourceFiles(source_file_filter):
    if 'third_party' in affected_file.LocalPath():
      continue
    contents = input_api.ReadFile(affected_file, 'rb')
    if not re.search(copyright_pattern, contents):
      results.append(output_api.PresubmitError(
          '%s is missing a correct copyright header.' % affected_file))
  return results


def _ToolFlags(input_api, output_api):
  """Make sure `{dm,nanobench}_flags.py test` passes if modified."""
  results = []
  sources = lambda x: ('dm_flags.py'        in x.LocalPath() or
                       'nanobench_flags.py' in x.LocalPath())
  for f in input_api.AffectedSourceFiles(sources):
    if 0 != subprocess.call(['python', f.LocalPath(), 'test']):
      results.append(output_api.PresubmitError('`python %s test` failed' % f))
  return results


def _CommonChecks(input_api, output_api):
  """Presubmit checks common to upload and commit."""
  results = []
  sources = lambda x: (x.LocalPath().endswith('.h') or
                       x.LocalPath().endswith('.gypi') or
                       x.LocalPath().endswith('.gyp') or
                       x.LocalPath().endswith('.py') or
                       x.LocalPath().endswith('.sh') or
                       x.LocalPath().endswith('.m') or
                       x.LocalPath().endswith('.mm') or
                       x.LocalPath().endswith('.go') or
                       x.LocalPath().endswith('.c') or
                       x.LocalPath().endswith('.cc') or
                       x.LocalPath().endswith('.cpp'))
  results.extend(
      _CheckChangeHasEol(
          input_api, output_api, source_file_filter=sources))
  results.extend(_PythonChecks(input_api, output_api))
  results.extend(_IfDefChecks(input_api, output_api))
  results.extend(_CopyrightChecks(input_api, output_api,
                                  source_file_filter=sources))
  results.extend(_ToolFlags(input_api, output_api))
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
  for affected_file in input_api.AffectedFiles():
    affected_file_path = affected_file.LocalPath()
    file_path, file_ext = os.path.splitext(affected_file_path)
    # We only care about files that end in .h and are under the top-level
    # include dir, but not include/private.
    if (file_ext == '.h' and
        'include' == file_path.split(os.path.sep)[0] and
        'private' not in file_path):
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

    # TODO(rmistry): Stop checking for COMMIT=false once crbug/470609 is
    # resolved.
    if issue_properties['cq_dry_run'] or re.search(
        r'^COMMIT=false$', issue_properties['description'], re.M):
      # Ignore public api owners check for dry run CLs since they are not
      # going to be committed.
      return results

    match = re.search(r'^TBR=(.*)$', issue_properties['description'], re.M)
    if match:
      tbr_entries = match.group(1).strip().split(',')
      for owner in PUBLIC_API_OWNERS:
        if owner in tbr_entries or owner.split('@')[0] in tbr_entries:
          # If an owner is specified in the TBR= line then ignore the public
          # api owners check.
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
          break

  if not lgtm_from_owner:
    results.append(
        output_api.PresubmitError(
            "If this CL adds to or changes Skia's public API, you need an LGTM "
            "from any of %s.  If this CL only removes from or doesn't change "
            "Skia's public API, please add a short note to the CL saying so "
            "and add one of those reviewers on a TBR= line.  If you don't know "
            "if this CL affects Skia's public API, treat it like it does."
            % str(PUBLIC_API_OWNERS)))
  return results


def PostUploadHook(cl, change, output_api):
  """git cl upload will call this hook after the issue is created/modified.

  This hook does the following:
  * Adds a link to preview docs changes if there are any docs changes in the CL.
  * Adds 'NOTRY=true' if the CL contains only docs changes.
  * Adds 'NOTREECHECKS=true' for non master branch changes since they do not
    need to be gated on the master branch's tree.
  * Adds 'NOTRY=true' for non master branch changes since trybots do not yet
    work on them.
  * Adds 'NOPRESUBMIT=true' for non master branch changes since those don't
    run the presubmit checks.
  """

  results = []
  atleast_one_docs_change = False
  all_docs_changes = True
  for affected_file in change.AffectedFiles():
    affected_file_path = affected_file.LocalPath()
    file_path, _ = os.path.splitext(affected_file_path)
    if 'site' == file_path.split(os.path.sep)[0]:
      atleast_one_docs_change = True
    else:
      all_docs_changes = False
    if atleast_one_docs_change and not all_docs_changes:
      break

  issue = cl.issue
  rietveld_obj = cl.RpcServer()
  if issue and rietveld_obj:
    original_description = rietveld_obj.get_description(issue)
    new_description = original_description

    # If the change includes only doc changes then add NOTRY=true in the
    # CL's description if it does not exist yet.
    if all_docs_changes and not re.search(
        r'^NOTRY=true$', new_description, re.M | re.I):
      new_description += '\nNOTRY=true'
      results.append(
          output_api.PresubmitNotifyResult(
              'This change has only doc changes. Automatically added '
              '\'NOTRY=true\' to the CL\'s description'))

    # If there is atleast one docs change then add preview link in the CL's
    # description if it does not already exist there.
    if atleast_one_docs_change and not re.search(
        r'^DOCS_PREVIEW=.*', new_description, re.M | re.I):
      # Automatically add a link to where the docs can be previewed.
      new_description += '\nDOCS_PREVIEW= %s%s' % (DOCS_PREVIEW_URL, issue)
      results.append(
          output_api.PresubmitNotifyResult(
              'Automatically added a link to preview the docs changes to the '
              'CL\'s description'))

    # If the target ref is not master then add NOTREECHECKS=true and NOTRY=true
    # to the CL's description if it does not already exist there.
    target_ref = rietveld_obj.get_issue_properties(issue, False).get(
        'target_ref', '')
    if target_ref != 'refs/heads/master':
      if not re.search(
          r'^NOTREECHECKS=true$', new_description, re.M | re.I):
        new_description += "\nNOTREECHECKS=true"
        results.append(
            output_api.PresubmitNotifyResult(
                'Branch changes do not need to rely on the master branch\'s '
                'tree status. Automatically added \'NOTREECHECKS=true\' to the '
                'CL\'s description'))
      if not re.search(
          r'^NOTRY=true$', new_description, re.M | re.I):
        new_description += "\nNOTRY=true"
        results.append(
            output_api.PresubmitNotifyResult(
                'Trybots do not yet work for non-master branches. '
                'Automatically added \'NOTRY=true\' to the CL\'s description'))
      if not re.search(
          r'^NOPRESUBMIT=true$', new_description, re.M | re.I):
        new_description += "\nNOPRESUBMIT=true"
        results.append(
            output_api.PresubmitNotifyResult(
                'Branch changes do not run the presubmit checks.'))

    # Read and process the HASHTAGS file.
    hashtags_fullpath = os.path.join(change._local_root, 'HASHTAGS')
    with open(hashtags_fullpath, 'rb') as hashtags_csv:
      hashtags_reader = csv.reader(hashtags_csv, delimiter=',')
      for row in hashtags_reader:
        if not row or row[0].startswith('#'):
          # Ignore empty lines and comments
          continue
        hashtag = row[0]
        # Search for the hashtag in the description.
        if re.search('#%s' % hashtag, new_description, re.M | re.I):
          for mapped_text in row[1:]:
            # Special case handling for CQ_KEYWORDS_THAT_NEED_APPENDING.
            appended_description = _HandleAppendingCQKeywords(
                hashtag, mapped_text, new_description, results, output_api)
            if appended_description:
              new_description = appended_description
              continue

            # Add the mapped text if it does not already exist in the
            # CL's description.
            if not re.search(
                r'^%s$' % mapped_text, new_description, re.M | re.I):
              new_description += '\n%s' % mapped_text
              results.append(
                  output_api.PresubmitNotifyResult(
                      'Found \'#%s\', automatically added \'%s\' to the CL\'s '
                      'description' % (hashtag, mapped_text)))

    # If the description has changed update it.
    if new_description != original_description:
      rietveld_obj.update_description(issue, new_description)

    return results


def _HandleAppendingCQKeywords(hashtag, keyword_and_value, description,
                               results, output_api):
  """Handles the CQ keywords that need appending if specified in hashtags."""
  keyword = keyword_and_value.split('=')[0]
  if keyword in CQ_KEYWORDS_THAT_NEED_APPENDING:
    # If the keyword is already in the description then append to it.
    match = re.search(
        r'^%s=(.*)$' % keyword, description, re.M | re.I)
    if match:
      old_values = match.group(1).split(';')
      new_value = keyword_and_value.split('=')[1]
      if new_value in old_values:
        # Do not need to do anything here.
        return description
      # Update the description with the new values.
      new_description = description.replace(
          match.group(0), "%s;%s" % (match.group(0), new_value))
      results.append(
          output_api.PresubmitNotifyResult(
          'Found \'#%s\', automatically appended \'%s\' to %s in '
          'the CL\'s description' % (hashtag, new_value, keyword)))
      return new_description
  return None


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
