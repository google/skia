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

class CheckUnsafeBuffersSafetyCommentsTest(unittest.TestCase):
    def _run(self, contents, path='src/core/SkFoo.cpp'):
        mock_input_api = MockInputApi()
        mock_input_api.files = [MockFile(path, contents)]
        return PRESUBMIT._CheckUnsafeBuffersSafetyComments(
            mock_input_api, MockOutputApi())

    def testNoUsage(self):
        results = self._run([
            'void Foo() {',
            '    int x = 0;',
            '}',
        ])
        self.assertEqual(0, len(results))

    def testValidUsageSameLine(self):
        results = self._run([
            'void Foo() {',
            '    SK_UNSAFE_BUFFERS(ptr[0] = 0); // SAFETY: safe',
            '}',
        ])
        self.assertEqual(0, len(results))

    def testValidUsagePrecedingLine(self):
        results = self._run([
            'void Foo() {',
            '    // SAFETY: safe',
            '    SK_UNSAFE_BUFFERS(ptr[0] = 0);',
            '}',
        ])
        self.assertEqual(0, len(results))

    def testValidUsagePrecedingLineWithOtherComments(self):
        results = self._run([
            'void Foo() {',
            '    // SAFETY: safe',
            '    // some other info',
            '    SK_UNSAFE_BUFFERS(ptr[0] = 0);',
            '}',
        ])
        self.assertEqual(0, len(results))

    def testValidUsageBrokenLine(self):
        results = self._run([
            'void Foo() {',
            '    // SAFETY: safe',
            '    int very_long_line_due_lots_of_stuff = line_broken_by_clang_format +',
            '    SK_UNSAFE_BUFFERS(ptr[0] = 0);',
            '}',
        ])
        self.assertEqual(0, len(results))

    def testValidUsageBrokenLineWithTrailingComment(self):
        results = self._run([
            'void Foo() {',
            '    // SAFETY: safe',
            '    int total = base +  // offset',
            '                SK_UNSAFE_BUFFERS(ptr[0]);',
            '}',
        ])
        self.assertEqual(0, len(results))

    def testValidUsageSafetyCommentWithCodeSyntax(self):
        results = self._run([
            'void Foo() {',
            '    // SAFETY: Explanation too large to display in one line of comment',
            '    // and use of c++ syntax to justify usage. This is safe because',
            '    // dummy[1] = dummy[0]++; is safe',
            '    SK_UNSAFE_BUFFERS(ptr[0] = 0);',
            '}',
        ])
        self.assertEqual(0, len(results))

    def testValidUsageInHeader(self):
        results = self._run([
            'inline void Foo() {',
            '    // SAFETY: safe',
            '    SK_UNSAFE_BUFFERS(ptr[0] = 0);',
            '}',
        ], path='include/private/SkFoo.h')
        self.assertEqual(0, len(results))

    def testNonSourceFileIgnored(self):
        results = self._run([
            'SK_UNSAFE_BUFFERS(ptr[0] = 0);',
        ], path='docs/examples/foo.md')
        self.assertEqual(0, len(results))

    def testInvalidUsageCommentSeparatedByForLoop(self):
        results = self._run([
            'void Foo() {',
            '    // SAFETY: safe',
            '    for (int b = 0; b < buffer_size; b++) {',
            '        SK_UNSAFE_BUFFERS(dummy[b] += x);',
            '    }',
            '}',
        ])
        self.assertEqual(1, len(results))

    def testInvalidUsageNoComment(self):
        results = self._run([
            'void Foo() {',
            '    SK_UNSAFE_BUFFERS(ptr[0] = 0);',
            '}',
        ])
        self.assertEqual(1, len(results))
        self.assertIsInstance(
            results[0], MockOutputApi.PresubmitPromptWarning, 'Not a warning')
        self.assertIn('SK_UNSAFE_BUFFERS usage must be accompanied',
                      results[0].items[0])
        self.assertIn('src/core/SkFoo.cpp:2:', results[0].items[0])

    def testInvalidUsageCommentNotSafety(self):
        results = self._run([
            'void Foo() {',
            '    // this is a comment but not safety',
            '    SK_UNSAFE_BUFFERS(ptr[0] = 0);',
            '}',
        ])
        self.assertEqual(1, len(results))

    def testInvalidUsageCommentSeparatedByCode(self):
        results = self._run([
            'void Foo() {',
            '    // SAFETY: safe',
            '    int x = 0;',
            '    SK_UNSAFE_BUFFERS(ptr[0] = 0);',
            '}',
        ])
        self.assertEqual(1, len(results))

    def testInvalidUsageCommentSeparatedByCodeWithTrailingComment(self):
        results = self._run([
            'void Foo() {',
            '    // SAFETY: safe',
            '    int x = 0;  // init',
            '    SK_UNSAFE_BUFFERS(ptr[0] = 0);',
            '}',
        ])
        self.assertEqual(1, len(results))

    def testInvalidUsageCommentSeparatedByCodeWithSlashesInString(self):
        results = self._run([
            'void Foo() {',
            '    // SAFETY: safe',
            '    const char* kUrl = "https://skia.org";',
            '    SK_UNSAFE_BUFFERS(ptr[0] = 0);',
            '}',
        ])
        self.assertEqual(1, len(results))

    def testIgnoreCommentedUsage(self):
        results = self._run([
            'void Foo() {',
            '    // SK_UNSAFE_BUFFERS(ptr[0] = 0);',
            '}',
        ])
        self.assertEqual(0, len(results))

    def testUnchangedUsageIgnored(self):
        contents = [
            'void Foo() {',
            '    SK_UNSAFE_BUFFERS(ptr[0] = 0);',
            '    int x = 0;',
            '}',
        ]
        mock_file = MockFile('src/core/SkFoo.cpp', contents)
        # Only line 3 was changed; the pre-existing usage on line 2 is ignored.
        mock_file._changed_contents = [(3, contents[2])]
        mock_input_api = MockInputApi()
        mock_input_api.files = [mock_file]
        results = PRESUBMIT._CheckUnsafeBuffersSafetyComments(
            mock_input_api, MockOutputApi())
        self.assertEqual(0, len(results))

    def testSafetyCommentOnUnchangedLine(self):
        contents = [
            'void Foo() {',
            '    // SAFETY: safe',
            '    SK_UNSAFE_BUFFERS(ptr[0] = 0);',
            '}',
        ]
        mock_file = MockFile('src/core/SkFoo.cpp', contents)
        # Only the usage line changed; the SAFETY comment above it is old.
        mock_file._changed_contents = [(3, contents[2])]
        mock_input_api = MockInputApi()
        mock_input_api.files = [mock_file]
        results = PRESUBMIT._CheckUnsafeBuffersSafetyComments(
            mock_input_api, MockOutputApi())
        self.assertEqual(0, len(results))


class CheckNoBannedPatternsTest(unittest.TestCase):
    def _run(self, contents, path='src/core/SkFoo.cpp'):
        mock_input_api = MockInputApi()
        mock_input_api.files = [MockFile(path, contents)]
        return PRESUBMIT._CheckNoBannedPatterns(
            mock_input_api, MockOutputApi())

    def testNoUsage(self):
        results = self._run([
            'void Foo() {',
            '    int x = 0;',
            '}',
        ])
        self.assertEqual(0, len(results))

    def testBannedUnsafeTodo(self):
        results = self._run([
            'void Foo() {',
            '    SK_UNSAFE_TODO(ptr[0] = 0);',
            '}',
        ])
        self.assertEqual(1, len(results))
        self.assertIsInstance(
            results[0], MockOutputApi.PresubmitPromptWarning, 'Not a warning')
        self.assertIn('A banned pattern was used.', results[0].message)
        self.assertIn('src/core/SkFoo.cpp:2:', results[0].message)
        self.assertIn('Do not introduce new instances of SK_UNSAFE_TODO',
                      results[0].message)
        self.assertEqual(1, len(results[0].locations))
        loc = results[0].locations[0]
        self.assertEqual('src/core/SkFoo.cpp', loc.file_path)
        self.assertEqual(2, loc.start_line)
        self.assertEqual(2, loc.end_line)

    def testCommentedUsageIgnored(self):
        results = self._run([
            '// SK_UNSAFE_TODO(ptr[0] = 0);',
        ])
        self.assertEqual(0, len(results))

    def testNoCheckBypass(self):
        results = self._run([
            '    SK_UNSAFE_TODO(ptr[0] = 0); // nocheck',
        ])
        self.assertEqual(0, len(results))

    def testSubstringNotMatched(self):
        results = self._run([
            '    SK_UNSAFE_TODOISH(ptr[0] = 0);',
        ])
        self.assertEqual(0, len(results))

    def testNonSourceFileIgnored(self):
        results = self._run([
            'SK_UNSAFE_TODO(ptr[0] = 0);',
        ], path='tools/foo.py')
        self.assertEqual(0, len(results))

    def testSafeBuffersMacroNotBanned(self):
        results = self._run([
            '    // SAFETY: safe',
            '    SK_UNSAFE_BUFFERS(ptr[0] = 0);',
        ])
        self.assertEqual(0, len(results))


if __name__ == '__main__':
    unittest.main()
