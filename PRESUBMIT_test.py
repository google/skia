#!/usr/bin/env python3
# Copyright 2023 Google LLC
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest
from unittest import mock

import PRESUBMIT

from PRESUBMIT_test_mocks import MockFile, MockAffectedFile
from PRESUBMIT_test_mocks import MockInputApi, MockOutputApi


class ReleaseNotesTest(unittest.TestCase):
    def testNoEditTopReleaseNotesNoWarning(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('README.chromium', ''),
        ]

        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckTopReleaseNotesChanged(
            mock_input_api, mock_output_api)

        self.assertEqual(0, len(results))

    def testUpdateTopReleaseNotesIssuesWarning(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('RELEASE_NOTES.md', ''),
        ]

        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckTopReleaseNotesChanged(
            mock_input_api, mock_output_api)

        self.assertEqual(1, len(results))
        self.assertIsInstance(
            results[0], mock_output_api.PresubmitPromptWarning, 'Not a warning')
        self.assertTrue(results[0].message.startswith(
            'Do not edit RELEASE_NOTES.md'))

    def testUpdateTopReleaseNotesNoWarning(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('RELEASE_NOTES.md', ''),
            MockFile('relnotes/deleted_note.md', ''),
        ]

        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckTopReleaseNotesChanged(
            mock_input_api, mock_output_api)

        self.assertEqual(0, len(results))

    def testUpdatePublicHeaderAndNoReleaseNoteGeneratesWarning(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('include/core/SkDrawable.h', ''),
        ]

        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckReleaseNotesForPublicAPI(
            mock_input_api, mock_output_api)

        self.assertEqual(1, len(results))
        self.assertIsInstance(
            results[0], mock_output_api.PresubmitPromptWarning, 'Not a warning')

    def testUpdatePublicHeaderAndReleaseNoteGeneratesNoWarning(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('include/core/SkDrawable.h', ''),
            MockFile('relnotes/new_note.md', ''),
        ]

        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckReleaseNotesForPublicAPI(
            mock_input_api, mock_output_api)

        self.assertEqual(0, len(results))


class RunCommandAndCheckDiffTest(unittest.TestCase):
    def setUp(self):
        self.foo_file = MockAffectedFile('foo.txt', new_contents=['foo'])
        self.bar_file = MockAffectedFile('bar.txt', new_contents=['bar'])

        self.mock_input_api = MockInputApi()
        self.mock_input_api.files = [self.foo_file, self.bar_file]
        self.mock_output_api = MockOutputApi()

    def setContents(self, file, contents):
        file._new_contents = contents

    @mock.patch('subprocess.check_output')
    def testNoChangesReturnsNoResults(self, mock_subprocess):
        results = PRESUBMIT._RunCommandAndCheckDiff(self.mock_output_api, [], [])
        self.assertEqual(results, [])

    @mock.patch('subprocess.check_output')
    def testChangingIrrelevantFilesReturnsNoResults(self, mock_subprocess):
        mock_subprocess.side_effect = lambda *args, **kwargs: self.setContents(self.bar_file, ['foo'])
        results = PRESUBMIT._RunCommandAndCheckDiff(
            self.mock_output_api, ['cmd'], [self.foo_file],
        )
        self.assertEqual(results, [])

    @mock.patch('subprocess.check_output')
    def testChangingRelevantFilesReturnsDiff(self, mock_subprocess):
        mock_subprocess.side_effect = lambda *args, **kwargs: self.setContents(self.foo_file, ['bar'])
        results = PRESUBMIT._RunCommandAndCheckDiff(
            self.mock_output_api, ['cmd'], [self.foo_file],
        )
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].message,
"""Diffs found after running "cmd":

--- foo.txt
+++ foo.txt
@@ -1 +1 @@
-foo
+bar

Please commit or discard the above changes.""")


class SKCMSTest(unittest.TestCase):
    def testModifyingSkcmsCcFails(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('modules/skcms/skcms.cc', ['void foo() {}']),
        ]
        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckSKCMS(mock_input_api, mock_output_api)

        self.assertEqual(1, len(results))
        self.assertIsInstance(
            results[0], mock_output_api.PresubmitError, 'Expected PresubmitError')
        self.assertIn('modules/skcms/skcms.cc', results[0].message)
        self.assertIn('https://skia.googlesource.com/skcms', results[0].message)

    def testModifyingSkcmsSrcHeaderFails(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('modules/skcms/src/skcms_internals.h', ['#define FOO 1']),
        ]
        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckSKCMS(mock_input_api, mock_output_api)

        self.assertEqual(1, len(results))
        self.assertIsInstance(
            results[0], mock_output_api.PresubmitError, 'Expected PresubmitError')
        self.assertIn('modules/skcms/src/skcms_internals.h', results[0].message)

    def testModifyingSkcmsSrcCcFails(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('modules/skcms/src/skcms_TransformBaseline.cc', ['void bar() {}']),
        ]
        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckSKCMS(mock_input_api, mock_output_api)

        self.assertEqual(1, len(results))
        self.assertIsInstance(
            results[0], mock_output_api.PresubmitError, 'Expected PresubmitError')
        self.assertIn('modules/skcms/src/skcms_TransformBaseline.cc', results[0].message)

    def testModifyingSkiaManagedSkcmsFilesPasses(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('modules/skcms/BUILD.bazel', ['# bazel']),
            MockFile('modules/skcms/BUILD.gn', ['# gn']),
            MockFile('modules/skcms/OWNERS', ['someone@google.com']),
            MockFile('modules/skcms/README.chromium', ['Revision: abc']),
            MockFile('modules/skcms/skcms.gni', ['# gni']),
            MockFile('modules/skcms/version.sha1', ['abc']),
        ]
        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckSKCMS(mock_input_api, mock_output_api)

        self.assertEqual(0, len(results))

    def testServiceAccountCanModifySkcms(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('modules/skcms/skcms.cc', ['void foo() {}']),
            MockFile('modules/skcms/src/skcms_internals.h', ['#define FOO 1']),
        ]
        mock_input_api.change.author_email = (
            'skia-autoroll@skia-public.iam.gserviceaccount.com'
        )
        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckSKCMS(mock_input_api, mock_output_api)

        self.assertEqual(0, len(results))


if __name__ == '__main__':
    unittest.main()
