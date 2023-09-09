// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package testutils

import (
	"archive/zip"
	"bytes"
	"os"
	"path/filepath"
	"sort"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/task_driver/go/td"
)

// AssertStepNames flattens the names of the steps in the given report, then asserts that they
// match the given list of strings.
func AssertStepNames(t *testing.T, report *td.StepReport, expectedStepNames ...string) {
	var actualStepNames []string
	report.Recurse(func(sr *td.StepReport) bool {
		if sr == report {
			// The root step created by td.RunTestSteps() is always "fake-test-task". We can omit it.
			return true
		}
		actualStepNames = append(actualStepNames, sr.Name)
		return true
	})
	assert.EqualValues(t, expectedStepNames, actualStepNames)
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

// PopulateDir populates a directory with the given contents, represented as a map from file paths
// to file contents.
func PopulateDir(t *testing.T, dirPath string, contents map[string]string) {
	for filePath, fileContents := range contents {
		require.NoError(t, os.WriteFile(filepath.Join(dirPath, filePath), []byte(fileContents), 0644))
	}
}

// MakeTempDirMockFn returns a function that can be used to mock os_steps.TempDir by setting the
// os_steps.TempDirContextKey context key. It takes a list of directory paths, and returns the
// first path on first call, then the second, and so on. If the function is called more times than
// the number of directories, the test fails.
func MakeTempDirMockFn(t *testing.T, dirs ...string) func(string, string) (string, error) {
	require.NotEmpty(t, dirs)
	i := 0
	return func(string, string) (string, error) {
		require.Less(t, i, len(dirs))
		dir := dirs[i]
		i++
		return dir, nil
	}
}
