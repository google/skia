#!/usr/bin/env python3
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
import subprocess
import sys
import traceback


RELEASE_NOTES_DIR = 'relnotes'
RELEASE_NOTES_FILE_NAME = 'RELEASE_NOTES.md'
RELEASE_NOTES_README = '//relnotes/README.md'

GOLD_TRYBOT_URL = 'https://gold.skia.org/search?issue='

SERVICE_ACCOUNT_SUFFIX = [
    '@%s.iam.gserviceaccount.com' % project for project in [
        'skia-buildbots.google.com', 'skia-swarming-bots', 'skia-public',
        'skia-corp.google.com', 'chops-service-accounts']]

USE_PYTHON3 = True


def _CheckChangeHasEol(input_api, output_api, source_file_filter=None):
  """Checks that files end with at least one \n (LF)."""
  eof_files = []
  for f in input_api.AffectedSourceFiles(source_file_filter):
    contents = input_api.ReadFile(f, 'rb')
    # Check that the file ends in at least one newline character.
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
      for line in f:
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
    if ('third_party/' in affected_file.LocalPath() or
        'tests/sksl/' in affected_file.LocalPath() or
        'bazel/rbe/' in affected_file.LocalPath() or
        'bazel/external/' in affected_file.LocalPath() or
        'bazel/exporter/interfaces/mocks/' in affected_file.LocalPath()):
      continue
    contents = input_api.ReadFile(affected_file, 'rb')
    if not re.search(copyright_pattern, contents):
      results.append(output_api.PresubmitError(
          '%s is missing a correct copyright header.' % affected_file))
  return results


def _InfraTests(input_api, output_api):
  """Run the infra tests."""
  results = []
  if not any(f.LocalPath().startswith('infra')
             for f in input_api.AffectedFiles()):
    return results

  cmd = ['python3', os.path.join('infra', 'bots', 'infra_tests.py')]
  try:
    subprocess.check_output(cmd)
  except subprocess.CalledProcessError as e:
    results.append(output_api.PresubmitError(
        '`%s` failed:\n%s' % (' '.join(cmd), e.output)))
  return results


def _CheckGNFormatted(input_api, output_api):
  """Make sure any .gn files we're changing have been formatted."""
  files = []
  for f in input_api.AffectedFiles(include_deletes=False):
    if (f.LocalPath().endswith('.gn') or
        f.LocalPath().endswith('.gni')):
      files.append(f)
  if not files:
    return []

  cmd = ['python3', os.path.join('bin', 'fetch-gn')]
  try:
    subprocess.check_output(cmd)
  except subprocess.CalledProcessError as e:
    return [output_api.PresubmitError(
        '`%s` failed:\n%s' % (' '.join(cmd), e.output))]

  results = []
  for f in files:
    gn = 'gn.exe' if 'win32' in sys.platform else 'gn'
    gn = os.path.join(input_api.PresubmitLocalPath(), 'bin', gn)
    cmd = [gn, 'format', '--dry-run', f.LocalPath()]
    try:
      subprocess.check_output(cmd)
    except subprocess.CalledProcessError:
      fix = 'bin/gn format ' + f.LocalPath()
      results.append(output_api.PresubmitError(
          '`%s` failed, try\n\t%s' % (' '.join(cmd), fix)))
  return results


def _CheckGitConflictMarkers(input_api, output_api):
  pattern = input_api.re.compile('^(?:<<<<<<<|>>>>>>>) |^=======$')
  results = []
  for f in input_api.AffectedFiles():
    for line_num, line in f.ChangedContents():
      if f.LocalPath().endswith('.md'):
        # First-level headers in markdown look a lot like version control
        # conflict markers. http://daringfireball.net/projects/markdown/basics
        continue
      if pattern.match(line):
        results.append(
            output_api.PresubmitError(
                'Git conflict markers found in %s:%d %s' % (
                    f.LocalPath(), line_num, line)))
  return results


def _CheckIncludesFormatted(input_api, output_api):
  """Make sure #includes in files we're changing have been formatted."""
  files = [str(f) for f in input_api.AffectedFiles() if f.Action() != 'D']
  cmd = ['python3',
         'tools/rewrite_includes.py',
         '--dry-run'] + files
  if 0 != subprocess.call(cmd):
    return [output_api.PresubmitError('`%s` failed' % ' '.join(cmd))]
  return []


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


def _RegenerateAllExamplesCPP(input_api, output_api):
  """Regenerates all_examples.cpp if an example was added or deleted."""
  if not any(f.LocalPath().startswith('docs/examples/')
             for f in input_api.AffectedFiles()):
    return []
  command_str = 'tools/fiddle/make_all_examples_cpp.py'
  cmd = ['python3', command_str]
  if 0 != subprocess.call(cmd):
    return [output_api.PresubmitError('`%s` failed' % ' '.join(cmd))]

  results = []
  git_diff_output = input_api.subprocess.check_output(
      ['git', 'diff', '--no-ext-diff'])
  if git_diff_output:
    results += [output_api.PresubmitError(
        'Diffs found after running "%s":\n\n%s\n'
        'Please commit or discard the above changes.' % (
            command_str,
            git_diff_output,
        )
    )]
  return results


def _CheckExamplesForPrivateAPIs(input_api, output_api):
  """We only want our checked-in examples (aka fiddles) to show public API."""
  banned_includes = [
    input_api.re.compile(r'#\s*include\s+("src/.*)'),
    input_api.re.compile(r'#\s*include\s+("include/private/.*)'),
  ]
  file_filter = lambda x: (x.LocalPath().startswith('docs/examples/'))
  errors = []
  for affected_file in input_api.AffectedSourceFiles(file_filter):
    affected_filepath = affected_file.LocalPath()
    for (line_num, line) in affected_file.ChangedContents():
      for re in banned_includes:
        match = re.search(line)
        if match:
          errors.append('%s:%s: Fiddles should not use private/internal API like %s.' % (
                affected_filepath, line_num, match.group(1)))

  if errors:
    return [output_api.PresubmitError('\n'.join(errors))]
  return []


def _CheckGeneratedBazelBUILDFiles(input_api, output_api):
    if 'win32' in sys.platform:
      # TODO(crbug.com/skia/12541): Remove when Bazel builds work on Windows.
      # Note: `make` is not installed on Windows by default.
      return []
    if 'darwin' in sys.platform:
      # This takes too long on Mac with default settings. Probably due to sandboxing.
      return []
    for affected_file in input_api.AffectedFiles(include_deletes=True):
      affected_file_path = affected_file.LocalPath()
      if (affected_file_path.endswith('.go') or
          affected_file_path.endswith('BUILD.bazel')):
        return _RunCommandAndCheckGitDiff(output_api,
                                          ['make', '-C', 'bazel', 'generate_go'])
    return []  # No modified Go source files.


def _CheckBazelBUILDFiles(input_api, output_api):
  """Makes sure our BUILD.bazel files are compatible with G3."""
  results = []
  for affected_file in input_api.AffectedFiles(include_deletes=False):
    affected_file_path = affected_file.LocalPath()
    is_bazel = affected_file_path.endswith('BUILD.bazel')
    # This list lines up with the one in autoroller_lib.py (see G3).
    excluded_paths = ["infra/", "bazel/rbe/", "bazel/external/", "bazel/common_config_settings/",
                      "modules/canvaskit/go/", "experimental/", "bazel/platform", "third_party/",
                      "tests/", "resources/", "bazel/deps_parser/", "bazel/exporter_tool/",
                      "tools/gpu/gl/interface/", "bazel/utils/", "include/config/",
                      "bench/", "example/external_client/"]
    is_excluded = any(affected_file_path.startswith(n) for n in excluded_paths)
    if is_bazel and not is_excluded:
      with open(affected_file_path, 'r') as file:
        contents = file.read()
        if 'exports_files_legacy(' not in contents:
          results.append(output_api.PresubmitError(
            ('%s needs to call exports_files_legacy() to support legacy G3 ' +
             'rules.\nPut this near the top of the file, beneath ' +
             'licenses(["notice"]).') % affected_file_path
          ))
        if 'licenses(["notice"])' not in contents:
          results.append(output_api.PresubmitError(
            ('%s needs to have\nlicenses(["notice"])\nimmediately after ' +
             'the load() calls to comply with G3 policies.') % affected_file_path
          ))
        if 'cc_library(' in contents and '"skia_cc_library"' not in contents:
          results.append(output_api.PresubmitError(
            ('%s needs to load skia_cc_library from macros.bzl instead of using the ' +
             'native one. This allows us to build differently for G3.\n' +
             'Add "skia_cc_library" to load("//bazel:macros.bzl", ...)')
            % affected_file_path
          ))
  return results


def _CheckPublicBzl(input_api, output_api):
  """Reminds devs to add/remove files from public.bzl."""
  results = []
  public_bzl = ''
  with open('public.bzl', 'r', encoding='utf-8') as f:
    public_bzl = f.read().strip()
  for affected_file in input_api.AffectedFiles(include_deletes=True):
    # action is A for newly added, D for newly deleted, M for modified
    action = affected_file.Action()
    affected_file_path = affected_file.LocalPath()
    if ((affected_file_path.startswith("include") or affected_file_path.startswith("src")) and
        (affected_file_path.endswith(".cpp") or affected_file_path.endswith(".h") or
         affected_file_path.endswith(".mm"))):
      affected_file_path = '"' + affected_file_path + '"'
      if action == "D" and affected_file_path in public_bzl:
        results.append(output_api.PresubmitError(
              "Need to delete %s from public.bzl (or rename it)" % affected_file_path))
      elif action == "A" and affected_file_path not in public_bzl:
        results.append(output_api.PresubmitPromptWarning(
              "You may need to add %s to public.bzl" % affected_file_path))
  return results


def _RunCommandAndCheckGitDiff(output_api, command):
  """Run an arbitrary command. Fail if it produces any diffs."""
  command_str = ' '.join(command)
  results = []

  try:
    output = subprocess.check_output(
        command,
        stderr=subprocess.STDOUT, encoding='utf-8')
  except subprocess.CalledProcessError as e:
    results += [output_api.PresubmitError(
        'Command "%s" returned non-zero exit code %d. Output: \n\n%s' % (
            command_str,
            e.returncode,
            e.output,
        )
    )]

  git_diff_output = subprocess.check_output(
      ['git', 'diff', '--no-ext-diff'], encoding='utf-8')
  if git_diff_output:
    results += [output_api.PresubmitError(
        'Diffs found after running "%s":\n\n%s\n'
        'Please commit or discard the above changes.' % (
            command_str,
            git_diff_output,
        )
    )]

  return results


def _CheckGNIGenerated(input_api, output_api):
  """Ensures that the generated *.gni files are current.

  The Bazel project files are authoritative and some *.gni files are
  generated from them using the exporter_tool. This check ensures they
  are still current.
  """
  if 'win32' in sys.platform:
    # TODO(crbug.com/skia/12541): Remove when Bazel builds work on Windows.
    # Note: `make` is not installed on Windows by default.
    return [
        output_api.PresubmitPromptWarning(
            'Skipping Bazel=>GNI export check on Windows (unsupported platform).'
        )
    ]
  if 'darwin' in sys.platform:
      # This takes too long on Mac with default settings. Probably due to sandboxing.
      return []
  should_run = False
  for affected_file in input_api.AffectedFiles(include_deletes=True):
    affected_file_path = affected_file.LocalPath()
    if affected_file_path.endswith('BUILD.bazel') or affected_file_path.endswith('.gni'):
      should_run = True
  # Generate GNI files and verify no changes.
  if should_run:
    return _RunCommandAndCheckGitDiff(output_api,
            ['make', '-C', 'bazel', 'generate_gni'])

  # No Bazel build files changed.
  return []


def _CheckBuildifier(input_api, output_api):
  """Runs Buildifier and fails on linting errors, or if it produces any diffs.

  This check only runs if the affected files include any WORKSPACE, BUILD,
  BUILD.bazel or *.bzl files.
  """
  files = []
  # Please keep the below exclude patterns in sync with those in the //:buildifier rule definition.
  for affected_file in input_api.AffectedFiles(include_deletes=False):
    affected_file_path = affected_file.LocalPath()
    if affected_file_path.endswith('BUILD.bazel') or affected_file_path.endswith('.bzl'):
      if not affected_file_path.endswith('public.bzl') and \
        not affected_file_path.endswith('go_repositories.bzl') and \
        not "bazel/rbe/gce_linux/" in affected_file_path and \
        not affected_file_path.startswith("third_party/externals/") and \
        not "node_modules/" in affected_file_path:  # Skip generated files.
        files.append(affected_file_path)
  if not files:
    return []
  try:
    subprocess.check_output(
        ['buildifier', '--version'],
        stderr=subprocess.STDOUT)
  except:
    return [output_api.PresubmitNotifyResult(
      'Skipping buildifier check because it is not on PATH. \n' +
      'You can download it from https://github.com/bazelbuild/buildtools/releases')]

  return _RunCommandAndCheckGitDiff(
    # One can change --lint=warn to --lint=fix to have things automatically fixed where possible.
    # However, --lint=fix will not cause a presubmit error if there are things that require
    # manual intervention, so we leave --lint=warn on by default.
    #
    # Please keep the below arguments in sync with those in the //:buildifier rule definition.
    output_api, [
      'buildifier',
      '--mode=fix',
      '--lint=warn',
      '--warnings',
      ','.join([
        '-native-android',
        '-native-cc',
        '-native-py',
      ])
    ] + files)


def _CheckBannedAPIs(input_api, output_api):
  """Check source code for functions and packages that should not be used."""

  # A list of tuples of a regex to match an API and a suggested replacement for
  # that API. There is an optional third parameter for files which *can* use this
  # API without warning.
  banned_replacements = [
    (r'std::stof\(', 'std::strtof(), which does not throw'),
    (r'std::stod\(', 'std::strtod(), which does not throw'),
    (r'std::stold\(', 'std::strtold(), which does not throw'),
  ]

  # These defines are either there or not, and using them with just an #if is a
  # subtle, frustrating bug.
  existence_defines = ['SK_GANESH', 'SK_GRAPHITE', 'SK_GL', 'SK_VULKAN', 'SK_DAWN', 'SK_METAL',
                       'SK_DIRECT3D', 'SK_DEBUG', 'GR_TEST_UTILS', 'GRAPHITE_TEST_UTILS']
  for d in existence_defines:
    banned_replacements.append(('#if {}'.format(d),
                                '#if defined({})'.format(d)))
  compiled_replacements = []
  for rep in banned_replacements:
    exceptions = []
    if len(rep) == 3:
      (re, replacement, exceptions) = rep
    else:
      (re, replacement) = rep

    compiled_re = input_api.re.compile(re)
    compiled_exceptions = [input_api.re.compile(exc) for exc in exceptions]
    compiled_replacements.append(
        (compiled_re, replacement, compiled_exceptions))

  errors = []
  file_filter = lambda x: (x.LocalPath().endswith('.h') or
                           x.LocalPath().endswith('.cpp') or
                           x.LocalPath().endswith('.cc') or
                           x.LocalPath().endswith('.m') or
                           x.LocalPath().endswith('.mm'))
  for affected_file in input_api.AffectedSourceFiles(file_filter):
    affected_filepath = affected_file.LocalPath()
    for (line_num, line) in affected_file.ChangedContents():
      for (re, replacement, exceptions) in compiled_replacements:
        match = re.search(line)
        if match:
          for exc in exceptions:
            if exc.search(affected_filepath):
              break
          else:
            errors.append('%s:%s: Instead of %s, please use %s.' % (
                affected_filepath, line_num, match.group(), replacement))

  if errors:
    return [output_api.PresubmitError('\n'.join(errors))]

  return []


def _CheckDEPS(input_api, output_api):
  """If DEPS was modified, run the deps_parser to update bazel/deps.bzl"""
  needs_running = False
  for affected_file in input_api.AffectedFiles(include_deletes=False):
    affected_file_path = affected_file.LocalPath()
    if affected_file_path.endswith('DEPS') or affected_file_path.endswith('deps.bzl'):
      needs_running = True
      break
  if not needs_running:
    return []
  try:
    subprocess.check_output(
        ['bazelisk', '--version'],
        stderr=subprocess.STDOUT)
  except:
    return [output_api.PresubmitNotifyResult(
      'Skipping DEPS check because bazelisk is not on PATH. \n' +
      'You can download it from https://github.com/bazelbuild/bazelisk/releases/tag/v1.14.0')]

  return _RunCommandAndCheckGitDiff(
    output_api, ['bazelisk', 'run', '//bazel/deps_parser'])


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
  results.extend(_CheckIncludesFormatted(input_api, output_api))
  results.extend(_CheckGNFormatted(input_api, output_api))
  results.extend(_CheckGitConflictMarkers(input_api, output_api))
  results.extend(_RegenerateAllExamplesCPP(input_api, output_api))
  results.extend(_CheckExamplesForPrivateAPIs(input_api, output_api))
  results.extend(_CheckBazelBUILDFiles(input_api, output_api))
  results.extend(_CheckBannedAPIs(input_api, output_api))
  return results


def CheckChangeOnUpload(input_api, output_api):
  """Presubmit checks for the change on upload."""
  results = []
  results.extend(_CommonChecks(input_api, output_api))
  # Run on upload, not commit, since the presubmit bot apparently doesn't have
  # coverage or Go installed.
  results.extend(_InfraTests(input_api, output_api))
  results.extend(_CheckTopReleaseNotesChanged(input_api, output_api))
  results.extend(_CheckReleaseNotesForPublicAPI(input_api, output_api))
  # Only check public.bzl on upload because new files are likely to be a source
  # of false positives and we don't want to unnecessarily block commits.
  results.extend(_CheckPublicBzl(input_api, output_api))
  # Buildifier might not be on the CI machines.
  results.extend(_CheckBuildifier(input_api, output_api))
  # We don't want this to block the CQ (for now).
  results.extend(_CheckDEPS(input_api, output_api))
  # Bazelisk is not yet included in the Presubmit job.
  results.extend(_CheckGeneratedBazelBUILDFiles(input_api, output_api))
  results.extend(_CheckGNIGenerated(input_api, output_api))
  return results


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


def _CheckReleaseNotesForPublicAPI(input_api, output_api):
  """Checks to see if a release notes file is added or edited with public API changes."""
  results = []
  public_api_changed = False
  release_file_changed = False
  for affected_file in input_api.AffectedFiles():
    affected_file_path = affected_file.LocalPath()
    file_path, file_ext = os.path.splitext(affected_file_path)
    # We only care about files that end in .h and are under the top-level
    # include dir, but not include/private.
    if (file_ext == '.h' and
        file_path.split(os.path.sep)[0] == 'include' and
        'private' not in file_path):
      public_api_changed = True
    elif os.path.dirname(file_path) == RELEASE_NOTES_DIR:
      release_file_changed = True

  if public_api_changed and not release_file_changed:
    results.append(output_api.PresubmitPromptWarning(
        'If this change affects a client API, please add a new summary '
        'file in the %s directory. More information can be found in '
        '%s.' % (RELEASE_NOTES_DIR, RELEASE_NOTES_README)))
  return results


def _CheckTopReleaseNotesChanged(input_api, output_api):
  """Warns if the top level release notes file was changed.

  The top level file is now auto-edited, and new release notes should
  be added to the RELEASE_NOTES_DIR directory"""
  results = []
  top_relnotes_changed = False
  release_file_changed = False
  for affected_file in input_api.AffectedFiles():
    affected_file_path = affected_file.LocalPath()
    file_path, file_ext = os.path.splitext(affected_file_path)
    if affected_file_path == RELEASE_NOTES_FILE_NAME:
      top_relnotes_changed = True
    elif os.path.dirname(file_path) == RELEASE_NOTES_DIR:
      release_file_changed = True
  # When relnotes_util is run it will modify RELEASE_NOTES_FILE_NAME
  # and delete the individual note files in RELEASE_NOTES_DIR.
  # So, if both paths are modified do not emit a warning.
  if top_relnotes_changed and not release_file_changed:
    results.append(output_api.PresubmitPromptWarning(
        'Do not edit %s directly. %s is automatically edited during the '
        'release process. Release notes should be added as new files in '
        'the %s directory. More information can be found in %s.' % (RELEASE_NOTES_FILE_NAME,
                                                                    RELEASE_NOTES_FILE_NAME,
                                                                    RELEASE_NOTES_DIR,
                                                                    RELEASE_NOTES_README)))
  return results


def PostUploadHook(gerrit, change, output_api):
  """git cl upload will call this hook after the issue is created/modified.

  This hook does the following:
  * Adds a link to preview docs changes if there are any docs changes in the CL.
  * Adds 'No-Try: true' if the CL contains only docs changes.
  """
  if not change.issue:
    return []

  # Skip PostUploadHooks for all auto-commit service account bots. New
  # patchsets (caused due to PostUploadHooks) invalidates the CQ+2 vote from
  # the "--use-commit-queue" flag to "git cl upload".
  for suffix in SERVICE_ACCOUNT_SUFFIX:
    if change.author_email.endswith(suffix):
      return []

  results = []
  at_least_one_docs_change = False
  all_docs_changes = True
  for affected_file in change.AffectedFiles():
    affected_file_path = affected_file.LocalPath()
    file_path, _ = os.path.splitext(affected_file_path)
    if 'site' == file_path.split(os.path.sep)[0]:
      at_least_one_docs_change = True
    else:
      all_docs_changes = False
    if at_least_one_docs_change and not all_docs_changes:
      break

  footers = change.GitFootersFromDescription()
  description_changed = False

  # If the change includes only doc changes then add No-Try: true in the
  # CL's description if it does not exist yet.
  if all_docs_changes and 'true' not in footers.get('No-Try', []):
    description_changed = True
    change.AddDescriptionFooter('No-Try', 'true')
    results.append(
        output_api.PresubmitNotifyResult(
            'This change has only doc changes. Automatically added '
            '\'No-Try: true\' to the CL\'s description'))

  # If the description has changed update it.
  if description_changed:
    gerrit.UpdateDescription(
        change.FullDescriptionText(), change.issue)

  return results


def CheckChangeOnCommit(input_api, output_api):
  """Presubmit checks for the change on commit."""
  results = []
  results.extend(_CommonChecks(input_api, output_api))
  # Checks for the presence of 'DO NOT''SUBMIT' in CL description and in
  # content of files.
  results.extend(
      input_api.canned_checks.CheckDoNotSubmit(input_api, output_api))
  return results
