#!/usr/bin/env python3
# Copyright 2023 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import io
import os.path
import subprocess
import textwrap
import unittest

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


if __name__ == '__main__':
    unittest.main()
