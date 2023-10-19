// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package common

import (
	"archive/zip"
	"context"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"regexp"
	"strings"

	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

// validBazelLabelRegexps represent valid, fully-qualified Bazel labels.
var validBazelLabelRegexps = []*regexp.Regexp{
	regexp.MustCompile(`^//:[a-zA-Z0-9_-]+$`),                  // Matches "//:foo".
	regexp.MustCompile(`^/(/[a-zA-Z0-9_-]+)+:[a-zA-Z0-9_-]+$`), // Matches "//foo:bar", "//foo/bar:baz", etc.
}

// ValidateLabelAndReturnOutputsZipPath validates the given Bazel label and returns the path within
// the checkout directory where the ZIP archive with undeclared test outputs will be found, if
// applicable.
func ValidateLabelAndReturnOutputsZipPath(checkoutDir, label string) (string, error) {
	valid := false
	for _, re := range validBazelLabelRegexps {
		if re.MatchString(label) {
			valid = true
			break
		}
	}
	if !valid {
		return "", skerr.Fmt("invalid label: %q", label)
	}

	return filepath.Join(
		checkoutDir,
		"bazel-testlogs",
		strings.ReplaceAll(strings.TrimPrefix(label, "//"), ":", "/"),
		"test.outputs",
		"outputs.zip"), nil
}

// ExtractOutputsZip extracts the undeclared outputs ZIP archive into a temporary directory, and
// returns the path to said directory.
func ExtractOutputsZip(ctx context.Context, outputsZipPath string) (string, error) {
	// Create extraction directory.
	extractionDir, err := os_steps.TempDir(ctx, "", "bazel-test-output-dir-*")
	if err != nil {
		return "", skerr.Wrap(err)
	}

	// Extract ZIP archive.
	if err := td.Do(ctx, td.Props(fmt.Sprintf("Extract undeclared outputs archive %s into %s", outputsZipPath, extractionDir)), func(ctx context.Context) error {
		outputsZip, err := zip.OpenReader(outputsZipPath)
		if err != nil {
			return skerr.Wrap(err)
		}
		defer util.Close(outputsZip)

		for _, file := range outputsZip.File {
			// Skip directories. We assume all output files are at the root directory of the archive.
			if file.FileInfo().IsDir() {
				if err := td.Do(ctx, td.Props(fmt.Sprintf("Not extracting subdirectory: %s", file.Name)), func(ctx context.Context) error { return nil }); err != nil {
					return skerr.Wrap(err)
				}
				continue
			}

			// Skip files within subdirectories.
			if strings.Contains(file.Name, "/") {
				if err := td.Do(ctx, td.Props(fmt.Sprintf("Not extracting file within subdirectory: %s", file.Name)), func(ctx context.Context) error { return nil }); err != nil {
					return skerr.Wrap(err)
				}
				continue
			}

			// Ignore anything that is not a PNG or JSON file.
			if !strings.HasSuffix(strings.ToLower(file.Name), ".png") && !strings.HasSuffix(strings.ToLower(file.Name), ".json") {
				if err := td.Do(ctx, td.Props(fmt.Sprintf("Not extracting non-PNG / non-JSON file: %s", file.Name)), func(ctx context.Context) error { return nil }); err != nil {
					return skerr.Wrap(err)
				}
				continue
			}

			// Extract file.
			if err := td.Do(ctx, td.Props(fmt.Sprintf("Extracting file: %s", file.Name)), func(ctx context.Context) error {
				reader, err := file.Open()
				if err != nil {
					return skerr.Wrap(err)
				}
				defer util.Close(reader)

				bytes, err := io.ReadAll(reader)
				if err != nil {
					return skerr.Wrap(err)
				}
				return skerr.Wrap(os.WriteFile(filepath.Join(extractionDir, file.Name), bytes, 0644))
			}); err != nil {
				return skerr.Wrap(err)
			}
		}

		return nil
	}); err != nil {
		return "", skerr.Wrap(err)
	}

	return extractionDir, nil
}
