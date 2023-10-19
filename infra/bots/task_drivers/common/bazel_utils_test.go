// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package common

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/testutils"
)

func TestValidateLabelAndReturnOutputZipPath_ValidLabel_Success(t *testing.T) {
	test := func(label, expected string) {
		t.Run(label, func(t *testing.T) {
			actual, err := ValidateLabelAndReturnOutputsZipPath("/path/to/skia", label)
			require.NoError(t, err)
			assert.Equal(t, expected, actual)
		})
	}

	test("//:foo", "/path/to/skia/bazel-testlogs/foo/test.outputs/outputs.zip")
	test("//foo:bar", "/path/to/skia/bazel-testlogs/foo/bar/test.outputs/outputs.zip")
	test("//foo/bar:baz", "/path/to/skia/bazel-testlogs/foo/bar/baz/test.outputs/outputs.zip")
	test("//foo/bar/baz:qux", "/path/to/skia/bazel-testlogs/foo/bar/baz/qux/test.outputs/outputs.zip")
}

func TestValidateLabelAndReturnOutputZipPath_InvalidLabel_Error(t *testing.T) {
	test := func(label string) {
		t.Run(label, func(t *testing.T) {
			_, err := ValidateLabelAndReturnOutputsZipPath("/path/to/skia", label)
			require.Error(t, err)
			assert.Contains(t, err.Error(), fmt.Sprintf("invalid label: %q", label))
		})
	}

	test("foo")
	test("/foo")
	test("//foo")
	test(":foo")
	test("/:foo")

	test("foo/bar")
	test("foo:bar")
	test("/foo/bar")
	test("/foo:bar")
	test(":foo/bar")
	test(":foo:bar")
	test("//foo/bar")

	test("foo/bar/baz")
	test("foo/bar:baz")
	test("foo:bar/baz")
	test("foo:bar:baz")
	test("/foo/bar/baz")
	test("/foo/bar:baz")
	test("/foo:bar/baz")
	test("/foo:bar:baz")
	test("//foo/bar/baz")
	test("//foo:bar/baz")
	test("//foo:bar:baz")
}

func TestExtractOutputsZip_Success(t *testing.T) {
	zipContents := map[string]string{
		// File contents are not important for this test; only paths and file extensions are taken into
		// account.
		"alfa.png":    "fake PNG file",
		"alfa.json":   "fake JSON file",
		"bravo.PNG":   "fake PNG",
		"bravo.JSON":  "fake JSON file",
		"charlie.txt": "fake TXT file", // Neither PNG nor JSON; should be ignored.
		// Subdirectories should be ignored.
		"delta/echo.png":  "fake PNG file",
		"delta/echo.json": "fake JSON file",
		"delta/echo.txt":  "fake JSON file",
	}
	zipPath := filepath.Join(t.TempDir(), "outputs.zip")
	testutils.MakeZIP(t, zipPath, zipContents)

	extractionDir := t.TempDir()
	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		ctx = context.WithValue(ctx, os_steps.TempDirContextKey, testutils.MakeTempDirMockFn(t, extractionDir))
		var err error
		extractionDir, err = ExtractOutputsZip(ctx, zipPath)
		require.NoError(t, err)
		return nil
	})
	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)
	testutils.AssertStepNames(t, res,
		"Creating TempDir",
		fmt.Sprintf("Extract undeclared outputs archive %s into %s", zipPath, extractionDir),
		"Extracting file: alfa.json",
		"Extracting file: alfa.png",
		"Extracting file: bravo.JSON",
		"Extracting file: bravo.PNG",
		"Not extracting non-PNG / non-JSON file: charlie.txt",
		"Not extracting file within subdirectory: delta/echo.json",
		"Not extracting file within subdirectory: delta/echo.png",
		"Not extracting file within subdirectory: delta/echo.txt")

	extractedFiles := map[string]bool{}
	files, err := os.ReadDir(extractionDir)
	require.NoError(t, err)
	for _, file := range files {
		assert.False(t, file.IsDir())
		extractedFiles[file.Name()] = true
	}

	assert.Equal(t, map[string]bool{
		"alfa.png":   true,
		"alfa.json":  true,
		"bravo.PNG":  true,
		"bravo.JSON": true,
	}, extractedFiles)
}
