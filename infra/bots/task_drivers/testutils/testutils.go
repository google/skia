// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package testutils

import (
	"archive/zip"
	"bytes"
	"os"
	"regexp"
	"sort"
	"testing"

	sk_exec "go.skia.org/infra/go/exec"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/task_driver/go/td"
)

// IgnoreCommandDir is a sentinel value that can be passed to AssertCommand to ignore the command's
// working directory.
const IgnoreCommandDir = ""

// AssertCommand asserts that the given command matches the given working directory, command name
// and arguments, which can be strings or regular expressions.
//
// Pass IgnoreCommandDir as the working directory to skip the working directory assertion.
func AssertCommand(t *testing.T, cmd *sk_exec.Command, dir, name string, args ...any) {
	if dir != IgnoreCommandDir {
		assert.Equal(t, dir, cmd.Dir)
	}
	assert.Equal(t, name, cmd.Name)
	AssertSliceMatchesStringsOrRegexps(t, cmd.Args, args...)
}

// AssertStepNamesMatchStringsOrRegexps flattens the names of the steps in the given report, then
// asserts that they match the given strings or regular expressions.
func AssertStepNamesMatchStringsOrRegexps(t *testing.T, report *td.StepReport, expectedStringsOrRegexps ...any) {
	var actualStepNames []string
	report.Recurse(func(sr *td.StepReport) bool {
		if sr == report {
			// The root step created by td.RunTestSteps() is always "fake-test-task". We can omit it.
			return true
		}
		actualStepNames = append(actualStepNames, sr.Name)
		return true
	})

	AssertSliceMatchesStringsOrRegexps(t, actualStepNames, expectedStringsOrRegexps...)
}

// AssertSliceMatchesStringsOrRegexps asserts that the elements in the given slice match the given
// strings or regular expressions.
func AssertSliceMatchesStringsOrRegexps(t *testing.T, actual []string, expectedStringsOrRegexps ...any) {
	require.Len(t, actual, len(expectedStringsOrRegexps), "actual slice: %#v", actual)
	for i, expected := range expectedStringsOrRegexps {
		switch stringOrRegexp := expected.(type) {
		case string:
			assert.Equal(t, stringOrRegexp, actual[i], "offending item index: %d", i)
		case *regexp.Regexp:
			assert.Regexp(t, stringOrRegexp, actual[i], "offending item index: %d", i)
		default:
			assert.Fail(t, "expected item is neither a string nor a regular expression", "expected item index: %d", i)
		}
	}
}

// MakeZIP creates a ZIP archive with the given contents, represented as a map from file paths to
// file contents.
func MakeZIP(t *testing.T, zipPath string, contents map[string]string) {
	buf := &bytes.Buffer{}
	zipWriter := zip.NewWriter(buf)

	// Sort files before adding them to the archive for determinism.
	var sortedFiles []string
	for file := range contents {
		sortedFiles = append(sortedFiles, file)
	}
	sort.Strings(sortedFiles)

	for _, file := range sortedFiles {
		fileWriter, err := zipWriter.Create(file)
		require.NoError(t, err)
		_, err = fileWriter.Write([]byte(contents[file]))
		require.NoError(t, err)
	}

	require.NoError(t, zipWriter.Close())
	require.NoError(t, os.WriteFile(zipPath, buf.Bytes(), 0644))
}
