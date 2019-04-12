# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Top-level presubmit script for Skia.

See http://dev.chromium.org/developers/how-tos/depottools/presubmit-scripts
for more details about the presubmit API built into gcl.
"""

import collections
import csv
import fnmatch
import os
import re
import subprocess
import sys
import traceback


REVERT_CL_SUBJECT_PREFIX = 'Revert '

SKIA_TREE_STATUS_URL = 'http://skia-tree-status.appspot.com'

# Please add the complete email address here (and not just 'xyz@' or 'xyz').
PUBLIC_API_OWNERS = (
    'reed@chromium.org',
    'reed@google.com',
    'bsalomon@chromium.org',
    'bsalomon@google.com',
    'djsollen@chromium.org',
    'djsollen@google.com',
    'hcm@chromium.org',
    'hcm@google.com',
)

AUTHORS_FILE_NAME = 'AUTHORS'

DOCS_PREVIEW_URL = 'https://skia.org/?cl='
GOLD_TRYBOT_URL = 'https://gold.skia.org/search?issue='

SERVICE_ACCOUNT_SUFFIX = [
    '@%s.iam.gserviceaccount.com' % project for project in [
        'skia-buildbots.google.com', 'skia-swarming-bots', 'skia-public',
        'skia-corp.google.com']]


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


def _JsonChecks(input_api, output_api):
  """Run checks on any modified json files."""
  failing_files = []
  for affected_file in input_api.AffectedFiles(None):
    affected_file_path = affected_file.LocalPath()
    is_json = affected_file_path.endswith('.json')
    is_metadata = (affected_file_path.startswith('site/') and
                   affected_file_path.endswith('/METADATA'))
    if is_json or is_metadata:
      try:
        input_api.json.load(open(affected_file_path, 'r'))
      except ValueError:
        failing_files.append(affected_file_path)

  results = []
  if failing_files:
    results.append(
        output_api.PresubmitError(
            'The following files contain invalid json:\n%s\n\n' %
                '\n'.join(failing_files)))
  return results


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
            'See https://bug.skia.org/3362 for why this should be fixed.' %
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


def _InfraTests(input_api, output_api):
  """Run the infra tests."""
  results = []
  if not any(f.LocalPath().startswith('infra')
             for f in input_api.AffectedFiles()):
    return results

  cmd = ['python', os.path.join('infra', 'bots', 'infra_tests.py')]
  try:
    subprocess.check_output(cmd)
  except subprocess.CalledProcessError as e:
    results.append(output_api.PresubmitError(
        '`%s` failed:\n%s' % (' '.join(cmd), e.output)))
  return results


def _CheckGNFormatted(input_api, output_api):
  """Make sure any .gn files we're changing have been formatted."""
  results = []
  for f in input_api.AffectedFiles():
    if (not f.LocalPath().endswith('.gn') and
        not f.LocalPath().endswith('.gni')):
      continue

    gn = 'gn.bat' if 'win32' in sys.platform else 'gn'
    cmd = [gn, 'format', '--dry-run', f.LocalPath()]
    try:
      subprocess.check_output(cmd)
    except subprocess.CalledProcessError:
      fix = 'gn format ' + f.LocalPath()
      results.append(output_api.PresubmitError(
          '`%s` failed, try\n\t%s' % (' '.join(cmd), fix)))
  return results


class _WarningsAsErrors():
  def __init__(self, output_api):
    self.output_api = output_api
    self.old_warning = None
  def __enter__(self):
    self.old_warning = self.output_api.PresubmitPromptWarning
    self.output_api.PresubmitPromptWarning = self.output_api.PresubmitError
    return self.output_api
  def __exit__(self, ex_type, ex_value, ex_traceback):
    self.output_api.PresubmitPromptWarning = self.old_warning


def _CommonChecks(input_api, output_api):
  """Presubmit checks common to upload and commit."""
  results = []
  sources = lambda x: (x.LocalPath().endswith('.h') or
                       x.LocalPath().endswith('.py') or
                       x.LocalPath().endswith('.sh') or
                       x.LocalPath().endswith('.m') or
                       x.LocalPath().endswith('.mm') or
                       x.LocalPath().endswith('.go') or
                       x.LocalPath().endswith('.c') or
                       x.LocalPath().endswith('.cc') or
                       x.LocalPath().endswith('.cpp'))
  results.extend(_CheckChangeHasEol(
      input_api, output_api, source_file_filter=sources))
  with _WarningsAsErrors(output_api):
    results.extend(input_api.canned_checks.CheckChangeHasNoCR(
        input_api, output_api, source_file_filter=sources))
    results.extend(input_api.canned_checks.CheckChangeHasNoStrayWhitespace(
        input_api, output_api, source_file_filter=sources))
  results.extend(_JsonChecks(input_api, output_api))
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
  # Run on upload, not commit, since the presubmit bot apparently doesn't have
  # coverage or Go installed.
  results.extend(_InfraTests(input_api, output_api))

  results.extend(_CheckGNFormatted(input_api, output_api))
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


class CodeReview(object):
  """Abstracts which codereview tool is used for the specified issue."""

  def __init__(self, input_api):
    self._issue = input_api.change.issue
    self._gerrit = input_api.gerrit

  def GetOwnerEmail(self):
    return self._gerrit.GetChangeOwner(self._issue)

  def GetSubject(self):
    return self._gerrit.GetChangeInfo(self._issue)['subject']

  def GetDescription(self):
    return self._gerrit.GetChangeDescription(self._issue)

  def IsDryRun(self):
    return self._gerrit.GetChangeInfo(
        self._issue)['labels']['Commit-Queue'].get('value', 0) == 1

  def GetReviewers(self):
    code_review_label = (
        self._gerrit.GetChangeInfo(self._issue)['labels']['Code-Review'])
    return [r['email'] for r in code_review_label.get('all', [])]

  def GetApprovers(self):
    approvers = []
    code_review_label = (
        self._gerrit.GetChangeInfo(self._issue)['labels']['Code-Review'])
    for m in code_review_label.get('all', []):
      if m.get("value") == 1:
        approvers.append(m["email"])
    return approvers


def _CheckOwnerIsInAuthorsFile(input_api, output_api):
  results = []
  if input_api.change.issue:
    cr = CodeReview(input_api)

    owner_email = cr.GetOwnerEmail()

    # Service accounts don't need to be in AUTHORS.
    for suffix in SERVICE_ACCOUNT_SUFFIX:
      if owner_email.endswith(suffix):
        return results

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
        results.append(
          output_api.PresubmitError(
            'The email %s is not in Skia\'s AUTHORS file.\n'
            'Issue owner, this CL must include an addition to the Skia AUTHORS '
            'file.'
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
  if input_api.change.issue:
    cr = CodeReview(input_api)

    if re.match(REVERT_CL_SUBJECT_PREFIX, cr.GetSubject(), re.I):
      # It is a revert CL, ignore the public api owners check.
      return results

    if cr.IsDryRun():
      # Ignore public api owners check for dry run CLs since they are not
      # going to be committed.
      return results

    if input_api.gerrit:
      for reviewer in cr.GetReviewers():
        if reviewer in PUBLIC_API_OWNERS:
          # If an owner is specified as an reviewer in Gerrit then ignore the
          # public api owners check.
          return results
    else:
      match = re.search(r'^TBR=(.*)$', cr.GetDescription(), re.M)
      if match:
        tbr_section = match.group(1).strip().split(' ')[0]
        tbr_entries = tbr_section.split(',')
        for owner in PUBLIC_API_OWNERS:
          if owner in tbr_entries or owner.split('@')[0] in tbr_entries:
            # If an owner is specified in the TBR= line then ignore the public
            # api owners check.
            return results

    if cr.GetOwnerEmail() in PUBLIC_API_OWNERS:
      # An owner created the CL that is an automatic LGTM.
      lgtm_from_owner = True

    for approver in cr.GetApprovers():
      if approver in PUBLIC_API_OWNERS:
        # Found an lgtm in a message from an owner.
        lgtm_from_owner = True
        break

  if not lgtm_from_owner:
    results.append(
        output_api.PresubmitError(
            "If this CL adds to or changes Skia's public API, you need an LGTM "
            "from any of %s.  If this CL only removes from or doesn't change "
            "Skia's public API, please add a short note to the CL saying so. "
            "Add one of the owners as a reviewer to your CL as well as to the "
            "TBR= line.  If you don't know if this CL affects Skia's public "
            "API, treat it like it does." % str(PUBLIC_API_OWNERS)))
  return results


def _FooterExists(footers, key, value):
  for k, v in footers:
    if k == key and v == value:
      return True
  return False


def PostUploadHook(cl, change, output_api):
  """git cl upload will call this hook after the issue is created/modified.

  This hook does the following:
  * Adds a link to preview docs changes if there are any docs changes in the CL.
  * Adds 'No-Try: true' if the CL contains only docs changes.
  * Adds 'No-Tree-Checks: true' for non master branch changes since they do not
    need to be gated on the master branch's tree.
  * Adds 'No-Presubmit: true' for non master branch changes since those don't
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
  if issue:
    # Skip PostUploadHooks for all auto-commit service account bots. New
    # patchsets (caused due to PostUploadHooks) invalidates the CQ+2 vote from
    # the "--use-commit-queue" flag to "git cl upload".
    for suffix in SERVICE_ACCOUNT_SUFFIX:
      if cl.GetIssueOwner().endswith(suffix):
        return results

    original_description_lines, footers = cl.GetDescriptionFooters()
    new_description_lines = list(original_description_lines)

    # If the change includes only doc changes then add No-Try: true in the
    # CL's description if it does not exist yet.
    if all_docs_changes and not _FooterExists(footers, 'No-Try', 'true'):
      new_description_lines.append('No-Try: true')
      results.append(
          output_api.PresubmitNotifyResult(
              'This change has only doc changes. Automatically added '
              '\'No-Try: true\' to the CL\'s description'))

    # If there is atleast one docs change then add preview link in the CL's
    # description if it does not already exist there.
    docs_preview_link = '%s%s' % (DOCS_PREVIEW_URL, issue)
    docs_preview_line = 'Docs-Preview: %s' % docs_preview_link
    if (atleast_one_docs_change and
        not _FooterExists(footers, 'Docs-Preview', docs_preview_link)):
      # Automatically add a link to where the docs can be previewed.
      new_description_lines.append(docs_preview_line)
      results.append(
          output_api.PresubmitNotifyResult(
              'Automatically added a link to preview the docs changes to the '
              'CL\'s description'))

    # If the target ref is not master then add 'No-Tree-Checks: true' and
    # 'No-Try: true' to the CL's description if it does not already exist there.
    target_ref = cl.GetRemoteBranch()[1]
    if target_ref != 'refs/remotes/origin/master':
      if not _FooterExists(footers, 'No-Tree-Checks', 'true'):
        new_description_lines.append('No-Tree-Checks: true')
        results.append(
            output_api.PresubmitNotifyResult(
                'Branch changes do not need to rely on the master branch\'s '
                'tree status. Automatically added \'No-Tree-Checks: true\' to '
                'the CL\'s description'))
      if not _FooterExists(footers, 'No-Presubmit', 'true'):
        new_description_lines.append('No-Presubmit: true')
        results.append(
            output_api.PresubmitNotifyResult(
                'Branch changes do not run the presubmit checks.'))

    # If the description has changed update it.
    if new_description_lines != original_description_lines:
      # Add a new line separating the new contents from the old contents.
      new_description_lines.insert(len(original_description_lines), '')
      cl.UpdateDescriptionFooters(new_description_lines, footers)

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
  # Checks for the presence of 'DO NOT''SUBMIT' in CL description and in
  # content of files.
  results.extend(
      input_api.canned_checks.CheckDoNotSubmit(input_api, output_api))
  return results
