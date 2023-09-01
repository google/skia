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
	"regexp"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/exec"
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

func TestUploadToGold_NoOutputsZip_Success(t *testing.T) {
	test := func(name string, utgArgs UploadToGoldArgs) {
		t.Run(name, func(t *testing.T) {
			commandCollector := exec.CommandCollector{}
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				ctx = td.WithExecRunFn(ctx, commandCollector.Run)

				err := UploadToGold(ctx, utgArgs, "/path/to/skia/bazel-testlogs/some/test/target/test.outputs/outputs.zip")

				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)
			testutils.AssertStepNamesMatchStringsOrRegexps(t, res,
				"Test did not produce an undeclared test outputs ZIP file; nothing to upload to Gold",
			)

			assert.Empty(t, commandCollector.Commands())
		})
	}

	test("post-submit task", UploadToGoldArgs{
		TestOnlyAllowAnyBazelLabel: true,
		BazelLabel:                 "//some/test:target",
		GoldctlPath:                "/path/to/goldctl",
		GitCommit:                  "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
	})

	test("CL task", UploadToGoldArgs{
		TestOnlyAllowAnyBazelLabel: true,
		BazelLabel:                 "//some/test:target",
		GoldctlPath:                "/path/to/goldctl",
		GitCommit:                  "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",

		// These arguments are only used when there are images to upload, and are therefore
		// ignored by the task driver under this test.
		ChangelistID:  "changelist-id",
		PatchsetOrder: "1",
		TryjobID:      "tryjob-id",
	})
}

func TestUploadToGold_WithOutputsZip_NoValidImages_NoGoldctlInvocations(t *testing.T) {
	test := func(name string, utgArgs UploadToGoldArgs) {
		t.Run(name, func(t *testing.T) {
			// Create fake archive with undeclared test outputs.
			outputsZipPath := filepath.Join(t.TempDir(), "bazel-testlogs", "some", "test", "target", "test.outputs", "outputs.zip")
			require.NoError(t, os.MkdirAll(filepath.Dir(outputsZipPath), 0700))
			testutils.MakeZIP(t, outputsZipPath, map[string]string{
				// The contents of PNG files does not matter for this test.
				"image-with-invalid-json-file.png": "fake PNG",
				"image-with-invalid-json-file.json": `{
					"invalid JSON file": "This JSON file should be ignored"
				}`,
				"image-with-no-json-file.png": "fake PNG",
				"json-file-with-no-image.json": `{
					"md5": "00000000000000000000000000000000",
					"keys": {
						"name": "no-image",
						"source_type": "no-corpus"
					}
				}`,
				"not-an-image-nor-json-file.txt": "I'm neither a PNG nor a JSON file.",
			})

			commandCollector := exec.CommandCollector{}
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				ctx = td.WithExecRunFn(ctx, commandCollector.Run)

				err := UploadToGold(ctx, utgArgs, outputsZipPath)

				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			testutils.AssertStepNamesMatchStringsOrRegexps(t, res,
				regexp.MustCompile("^Extract undeclared outputs archive .+/bazel-testlogs/some/test/target/test.outputs/outputs.zip into .+$"),
				"Extracting file: image-with-invalid-json-file.json",
				"Extracting file: image-with-invalid-json-file.png",
				"Extracting file: image-with-no-json-file.png",
				"Extracting file: json-file-with-no-image.json",
				"Ignoring non-PNG / non-JSON file: not-an-image-nor-json-file.txt",
				"Gather JSON and PNG files produced by GMs",
				"Ignoring \"image-with-invalid-json-file.json\": field \"md5\" not found",
				"Ignoring \"json-file-with-no-image.json\": file \"json-file-with-no-image.png\" not found",
				"Undeclared test outputs ZIP file contains no GM outputs; nothing to upload to Gold",
			)

			assert.Empty(t, commandCollector.Commands())
		})
	}

	test("post-submit task", UploadToGoldArgs{
		TestOnlyAllowAnyBazelLabel: true,
		BazelLabel:                 "//some/test:target",
		GoldctlPath:                "/path/to/goldctl",
		GitCommit:                  "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
	})

	test("CL task", UploadToGoldArgs{
		TestOnlyAllowAnyBazelLabel: true,
		BazelLabel:                 "//some/test:target",
		GoldctlPath:                "/path/to/goldctl",
		GitCommit:                  "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",

		// These arguments are only used when there are images to upload, and are therefore
		// ignored by the task driver under this test.
		ChangelistID:  "changelist-id",
		PatchsetOrder: "1",
		TryjobID:      "tryjob-id",
	})
}

func TestUploadToGold_WithOutputsZip_ValidImages_ImagesUploadedToGold(t *testing.T) {
	test := func(name string, utgArgs UploadToGoldArgs, goldctlImgtestInitStepName *regexp.Regexp, goldctlImgtestInitArgsStringsOrRegexps []any) {
		t.Run(name, func(t *testing.T) {
			// Create fake archive with undeclared test outputs.
			outputsZipPath := filepath.Join(t.TempDir(), "bazel-testlogs", "some", "test", "target", "test.outputs", "outputs.zip")
			require.NoError(t, os.MkdirAll(filepath.Dir(outputsZipPath), 0700))
			testutils.MakeZIP(t, outputsZipPath, map[string]string{
				// The contents of PNG files does not matter for this test.
				"alfa.png": "fake PNG",
				"alfa.json": `{
					"md5": "a01a01a01a01a01a01a01a01a01a01a0",
					"keys": {
						"build_system": "bazel",
						"name": "alfa",
						"source_type": "gm"
					}
				}`,
				"beta.png": "fake PNG",
				"beta.json": `{
					"md5": "b02b02b02b02b02b02b02b02b02b02b0",
					"keys": {
						"build_system": "bazel",
						"name": "beta",
						"source_type": "gm"
					}
				}`,
				"image-with-invalid-json-file.png": "fake PNG",
				"image-with-invalid-json-file.json": `{
					"invalid JSON file": "This JSON file should be ignored"
				}`,
				"image-with-no-json-file.png": "fake PNG",
				"json-file-with-no-image.json": `{
					"md5": "00000000000000000000000000000000",
					"keys": {
						"name": "no-image",
						"source_type": "no-corpus"
					}
				}`,
				"not-an-image-nor-json-file.txt": "I'm neither a PNG nor a JSON file.",
			})

			commandCollector := exec.CommandCollector{}
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				ctx = td.WithExecRunFn(ctx, commandCollector.Run)

				err := UploadToGold(ctx, utgArgs, outputsZipPath)

				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			testutils.AssertStepNamesMatchStringsOrRegexps(t, res,
				regexp.MustCompile("^Extract undeclared outputs archive .+/bazel-testlogs/some/test/target/test.outputs/outputs.zip into .+$"),
				"Extracting file: alfa.json",
				"Extracting file: alfa.png",
				"Extracting file: beta.json",
				"Extracting file: beta.png",
				"Extracting file: image-with-invalid-json-file.json",
				"Extracting file: image-with-invalid-json-file.png",
				"Extracting file: image-with-no-json-file.png",
				"Extracting file: json-file-with-no-image.json",
				"Ignoring non-PNG / non-JSON file: not-an-image-nor-json-file.txt",
				"Gather JSON and PNG files produced by GMs",
				"Gather \"alfa.png\"",
				"Gather \"beta.png\"",
				"Ignoring \"image-with-invalid-json-file.json\": field \"md5\" not found",
				"Ignoring \"json-file-with-no-image.json\": file \"json-file-with-no-image.png\" not found",
				"Upload GM outputs to Gold",
				"Creating TempDir",
				regexp.MustCompile("^/path/to/goldctl auth --work-dir [a-zA-Z0-9-/]+ --luci$"),
				goldctlImgtestInitStepName,
				regexp.MustCompile("^/path/to/goldctl imgtest add --work-dir [a-zA-Z0-9-/]+ --test-name alfa --png-file [a-zA-Z0-9-/]+/alfa.png --png-digest a01a01a01a01a01a01a01a01a01a01a0 --add-test-key build_system:bazel --add-test-key name:alfa --add-test-key source_type:gm$"),
				regexp.MustCompile("^/path/to/goldctl imgtest add --work-dir [a-zA-Z0-9-/]+ --test-name beta --png-file [a-zA-Z0-9-/]+/beta.png --png-digest b02b02b02b02b02b02b02b02b02b02b0 --add-test-key build_system:bazel --add-test-key name:beta --add-test-key source_type:gm$"),
				regexp.MustCompile("^/path/to/goldctl imgtest finalize --work-dir [a-zA-Z0-9-/]+$"),
			)

			require.Len(t, commandCollector.Commands(), 5)

			testutils.AssertCommand(t, commandCollector.Commands()[0],
				testutils.IgnoreCommandDir,
				"/path/to/goldctl",
				"auth",
				"--work-dir",
				regexp.MustCompile("^[a-zA-Z0-9-/]+$"),
				"--luci",
			)

			testutils.AssertCommand(t, commandCollector.Commands()[1],
				testutils.IgnoreCommandDir,
				"/path/to/goldctl",
				goldctlImgtestInitArgsStringsOrRegexps...,
			)

			testutils.AssertCommand(t, commandCollector.Commands()[2],
				testutils.IgnoreCommandDir,
				"/path/to/goldctl",
				"imgtest",
				"add",
				"--work-dir",
				regexp.MustCompile("^[a-zA-Z0-9-/]+$"),
				"--test-name",
				"alfa",
				"--png-file",
				regexp.MustCompile("[a-zA-Z0-9-/]+/alfa.png"),
				"--png-digest",
				"a01a01a01a01a01a01a01a01a01a01a0",
				"--add-test-key",
				"build_system:bazel",
				"--add-test-key",
				"name:alfa",
				"--add-test-key",
				"source_type:gm",
			)

			testutils.AssertCommand(t, commandCollector.Commands()[3],
				testutils.IgnoreCommandDir,
				"/path/to/goldctl",
				"imgtest",
				"add",
				"--work-dir",
				regexp.MustCompile("^[a-zA-Z0-9-/]+$"),
				"--test-name",
				"beta",
				"--png-file",
				regexp.MustCompile("^[a-zA-Z0-9-/]+/beta.png$"),
				"--png-digest",
				"b02b02b02b02b02b02b02b02b02b02b0",
				"--add-test-key",
				"build_system:bazel",
				"--add-test-key",
				"name:beta",
				"--add-test-key",
				"source_type:gm",
			)

			testutils.AssertCommand(t, commandCollector.Commands()[4],
				testutils.IgnoreCommandDir,
				"/path/to/goldctl",
				"imgtest",
				"finalize",
				"--work-dir",
				regexp.MustCompile("^[a-zA-Z0-9-/]+$"),
			)
		})
	}

	test(
		"post-submit task",
		UploadToGoldArgs{
			TestOnlyAllowAnyBazelLabel: true,
			BazelLabel:                 "//some/test:target",
			GoldctlPath:                "/path/to/goldctl",
			GitCommit:                  "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		},
		regexp.MustCompile("^/path/to/goldctl imgtest init --work-dir [a-zA-Z0-9-/]+ --instance skia --url https://gold.skia.org --bucket skia-infra-gm --git_hash ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99 --key os:linux$"),
		[]any{
			"imgtest",
			"init",
			"--work-dir",
			regexp.MustCompile("^[a-zA-Z0-9-/]+$"),
			"--instance",
			"skia",
			"--url",
			"https://gold.skia.org",
			"--bucket",
			"skia-infra-gm",
			"--git_hash",
			"ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
			"--key",
			"os:linux",
		},
	)

	test(
		"CL task",
		UploadToGoldArgs{
			TestOnlyAllowAnyBazelLabel: true,
			BazelLabel:                 "//some/test:target",
			GoldctlPath:                "/path/to/goldctl",
			GitCommit:                  "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
			ChangelistID:               "changelist-id",
			PatchsetOrder:              "1",
			TryjobID:                   "tryjob-id",
		},
		regexp.MustCompile("^/path/to/goldctl imgtest init --work-dir [a-zA-Z0-9-/]+ --instance skia --url https://gold.skia.org --bucket skia-infra-gm --git_hash ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99 --crs gerrit --cis buildbucket --changelist changelist-id --patchset 1 --jobid tryjob-id --key os:linux$"),
		[]any{
			"imgtest",
			"init",
			"--work-dir",
			regexp.MustCompile("^[a-zA-Z0-9-/]+$"),
			"--instance",
			"skia",
			"--url",
			"https://gold.skia.org",
			"--bucket",
			"skia-infra-gm",
			"--git_hash",
			"ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
			"--crs",
			"gerrit",
			"--cis",
			"buildbucket",
			"--changelist",
			"changelist-id",
			"--patchset",
			"1",
			"--jobid",
			"tryjob-id",
			"--key",
			"os:linux",
		},
	)
}
