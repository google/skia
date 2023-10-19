// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package common

import (
	"context"
	"os"
	"path/filepath"
	"testing"

	exec_testutils "go.skia.org/infra/go/exec/testutils"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/testutils"
)

func TestUploadToGold_NoOutputsZIPOrDir_NoGoldctlInvocations(t *testing.T) {
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
			testutils.AssertStepNames(t, res,
				"Test did not produce an undeclared test outputs ZIP file or directory; nothing to upload to Gold",
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

func TestUploadToGold_WithOutputsZIPOrDir_NoValidImages_NoGoldctlInvocations(t *testing.T) {
	test := func(name string, zip bool, utgArgs UploadToGoldArgs) {
		t.Run(name, func(t *testing.T) {
			undeclaredTestOutputs := map[string]string{
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
			}

			// Write undeclared test outputs to disk.
			outputsZIPOrDir := ""
			if zip {
				outputsZIPOrDir = filepath.Join(t.TempDir(), "bazel-testlogs", "some", "test", "target", "test.outputs", "outputs.zip")
				require.NoError(t, os.MkdirAll(filepath.Dir(outputsZIPOrDir), 0700))
				testutils.MakeZIP(t, outputsZIPOrDir, undeclaredTestOutputs)
			} else {
				outputsZIPOrDir = t.TempDir()
				testutils.PopulateDir(t, outputsZIPOrDir, undeclaredTestOutputs)
			}

			// Will be returned by the mocked os_steps.TempDir() when the task driver tries to create a
			// directory in which to extract the undeclared outputs ZIP archive.
			outputsZIPExtractionDir := t.TempDir()

			commandCollector := exec.CommandCollector{}
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				ctx = td.WithExecRunFn(ctx, commandCollector.Run)

				if zip {
					// Mock os_steps.TempDir() only for the case where outpusZIPOrDir is a ZIP archive. We
					// don't need to assert the exact number of times that os_steps.TempDir() is called
					// because said function produces a "Creating TempDir" task driver step, and we check the
					// exact set of steps produced.
					ctx = context.WithValue(ctx, os_steps.TempDirContextKey, testutils.MakeTempDirMockFn(t, outputsZIPExtractionDir))
				}

				err := UploadToGold(ctx, utgArgs, outputsZIPOrDir)

				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			expectedSteps := []string{}
			if zip {
				expectedSteps = append(expectedSteps,
					"Creating TempDir",
					"Extract undeclared outputs archive "+outputsZIPOrDir+" into "+outputsZIPExtractionDir,
					"Extracting file: image-with-invalid-json-file.json",
					"Extracting file: image-with-invalid-json-file.png",
					"Extracting file: image-with-no-json-file.png",
					"Extracting file: json-file-with-no-image.json",
					"Not extracting non-PNG / non-JSON file: not-an-image-nor-json-file.txt",
				)
			}
			expectedSteps = append(expectedSteps,
				"Gather JSON and PNG files produced by GMs",
				"Ignoring \"image-with-invalid-json-file.json\": field \"md5\" not found",
				"Ignoring \"json-file-with-no-image.json\": file \"json-file-with-no-image.png\" not found",
				"Undeclared test outputs ZIP file or directory contains no GM outputs; nothing to upload to Gold",
			)
			testutils.AssertStepNames(t, res, expectedSteps...)

			assert.Empty(t, commandCollector.Commands())
		})
	}

	postSubmitTaskArgs := UploadToGoldArgs{
		TestOnlyAllowAnyBazelLabel: true,
		BazelLabel:                 "//some/test:target",
		GoldctlPath:                "/path/to/goldctl",
		GitCommit:                  "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
	}
	test("post-submit task, ZIP file", true /* =zip */, postSubmitTaskArgs)
	test("post-submit task, directory", false /* =zip */, postSubmitTaskArgs)

	clTaskArgs := UploadToGoldArgs{
		TestOnlyAllowAnyBazelLabel: true,
		BazelLabel:                 "//some/test:target",
		GoldctlPath:                "/path/to/goldctl",
		GitCommit:                  "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",

		// These arguments are only used when there are images to upload, and are therefore
		// ignored by the task driver under this test.
		ChangelistID:  "changelist-id",
		PatchsetOrder: "1",
		TryjobID:      "tryjob-id",
	}
	test("CL task, ZIP file", true /* =zip */, clTaskArgs)
	test("CL task, directory", false /* =zip */, clTaskArgs)
}

func TestUploadToGold_WithOutputsZIPOrDir_ValidImages_ImagesUploadedToGold(t *testing.T) {
	test := func(name string, zip bool, utgArgs UploadToGoldArgs, goldctlWorkDir, goldctlImgtestInitStepName string, goldctlImgtestInitArgs []string) {
		t.Run(name, func(t *testing.T) {
			undeclaredTestOutputs := map[string]string{
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
			}

			// Write undeclared test outputs to disk.
			outpusZIPOrDir := ""
			if zip {
				outpusZIPOrDir = filepath.Join(t.TempDir(), "bazel-testlogs", "some", "test", "target", "test.outputs", "outputs.zip")
				require.NoError(t, os.MkdirAll(filepath.Dir(outpusZIPOrDir), 0700))
				testutils.MakeZIP(t, outpusZIPOrDir, undeclaredTestOutputs)
			} else {
				outpusZIPOrDir = t.TempDir()
				testutils.PopulateDir(t, outpusZIPOrDir, undeclaredTestOutputs)
			}

			// Will be returned by the mocked os_steps.TempDir() when the task driver tries to create a
			// directory in which to extract the undeclared outputs ZIP archive.
			outputsZIPExtractionDir := t.TempDir()

			commandCollector := exec.CommandCollector{}
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				ctx = td.WithExecRunFn(ctx, commandCollector.Run)

				tempDirMockFn := testutils.MakeTempDirMockFn(t, goldctlWorkDir)
				if zip {
					tempDirMockFn = testutils.MakeTempDirMockFn(t, outputsZIPExtractionDir, goldctlWorkDir)
				}
				// We don't need to assert the exact number of times that os_steps.TempDir() is called
				// because said function produces a "Creating TempDir" task driver step, and we check the
				// exact set of steps produced.
				ctx = context.WithValue(ctx, os_steps.TempDirContextKey, tempDirMockFn)

				err := UploadToGold(ctx, utgArgs, outpusZIPOrDir)

				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			dirWithTestOutputs := outpusZIPOrDir
			if zip {
				dirWithTestOutputs = outputsZIPExtractionDir
			}

			expectedSteps := []string{}
			if zip {
				expectedSteps = append(expectedSteps,
					"Creating TempDir",
					"Extract undeclared outputs archive "+outpusZIPOrDir+" into "+outputsZIPExtractionDir,
					"Extracting file: alfa.json",
					"Extracting file: alfa.png",
					"Extracting file: beta.json",
					"Extracting file: beta.png",
					"Extracting file: image-with-invalid-json-file.json",
					"Extracting file: image-with-invalid-json-file.png",
					"Extracting file: image-with-no-json-file.png",
					"Extracting file: json-file-with-no-image.json",
					"Not extracting non-PNG / non-JSON file: not-an-image-nor-json-file.txt",
				)
			}
			expectedSteps = append(expectedSteps,
				"Gather JSON and PNG files produced by GMs",
				"Gather \"alfa.png\"",
				"Gather \"beta.png\"",
				"Ignoring \"image-with-invalid-json-file.json\": field \"md5\" not found",
				"Ignoring \"json-file-with-no-image.json\": file \"json-file-with-no-image.png\" not found",
				"Upload GM outputs to Gold",
				"Creating TempDir",
				"/path/to/goldctl auth --work-dir "+goldctlWorkDir+" --luci",
				goldctlImgtestInitStepName,
				"/path/to/goldctl imgtest add --work-dir "+goldctlWorkDir+" --test-name alfa --png-file "+dirWithTestOutputs+"/alfa.png --png-digest a01a01a01a01a01a01a01a01a01a01a0 --add-test-key build_system:bazel --add-test-key name:alfa --add-test-key source_type:gm",
				"/path/to/goldctl imgtest add --work-dir "+goldctlWorkDir+" --test-name beta --png-file "+dirWithTestOutputs+"/beta.png --png-digest b02b02b02b02b02b02b02b02b02b02b0 --add-test-key build_system:bazel --add-test-key name:beta --add-test-key source_type:gm",
				"/path/to/goldctl imgtest finalize --work-dir "+goldctlWorkDir,
			)
			testutils.AssertStepNames(t, res, expectedSteps...)

			exec_testutils.AssertCommandsMatch(t, [][]string{
				{
					"/path/to/goldctl",
					"auth",
					"--work-dir", goldctlWorkDir,
					"--luci",
				},
				append([]string{"/path/to/goldctl"}, goldctlImgtestInitArgs...,
				),
				{
					"/path/to/goldctl",
					"imgtest",
					"add",
					"--work-dir", goldctlWorkDir,
					"--test-name", "alfa",
					"--png-file", dirWithTestOutputs + "/alfa.png",
					"--png-digest", "a01a01a01a01a01a01a01a01a01a01a0",
					"--add-test-key", "build_system:bazel",
					"--add-test-key", "name:alfa",
					"--add-test-key", "source_type:gm",
				},
				{
					"/path/to/goldctl",
					"imgtest",
					"add",
					"--work-dir", goldctlWorkDir,
					"--test-name", "beta",
					"--png-file", dirWithTestOutputs + "/beta.png",
					"--png-digest", "b02b02b02b02b02b02b02b02b02b02b0",
					"--add-test-key", "build_system:bazel",
					"--add-test-key", "name:beta",
					"--add-test-key", "source_type:gm",
				},
				{
					"/path/to/goldctl",
					"imgtest",
					"finalize",
					"--work-dir", goldctlWorkDir,
				},
			}, commandCollector.Commands())

		})
	}

	goldctlWorkDir := t.TempDir()
	postSubmitTaskArgs := UploadToGoldArgs{
		TestOnlyAllowAnyBazelLabel: true,
		BazelLabel:                 "//some/test:target",
		GoldctlPath:                "/path/to/goldctl",
		GitCommit:                  "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
	}
	postSubmitTaskGoldctlImgtestInitStep := "/path/to/goldctl imgtest init --work-dir " + goldctlWorkDir + " --instance skia --url https://gold.skia.org --bucket skia-infra-gm --git_hash ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99 --key os:linux"
	postSubmitTaskGoldctlImgtestInitArgs := []string{
		"imgtest",
		"init",
		"--work-dir", goldctlWorkDir,
		"--instance", "skia",
		"--url", "https://gold.skia.org",
		"--bucket", "skia-infra-gm",
		"--git_hash", "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		"--key", "os:linux",
	}
	test("post-submit task, ZIP file", true /* =zip */, postSubmitTaskArgs, goldctlWorkDir, postSubmitTaskGoldctlImgtestInitStep, postSubmitTaskGoldctlImgtestInitArgs)
	test("post-submit task, directory", false /* =zip */, postSubmitTaskArgs, goldctlWorkDir, postSubmitTaskGoldctlImgtestInitStep, postSubmitTaskGoldctlImgtestInitArgs)

	goldctlWorkDir = t.TempDir()
	clTaskArgs := UploadToGoldArgs{
		TestOnlyAllowAnyBazelLabel: true,
		BazelLabel:                 "//some/test:target",
		GoldctlPath:                "/path/to/goldctl",
		GitCommit:                  "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		ChangelistID:               "changelist-id",
		PatchsetOrder:              "1",
		TryjobID:                   "tryjob-id",
	}
	clTaskGoldctlImgtestInitStep := "/path/to/goldctl imgtest init --work-dir " + goldctlWorkDir + " --instance skia --url https://gold.skia.org --bucket skia-infra-gm --git_hash ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99 --crs gerrit --cis buildbucket --changelist changelist-id --patchset 1 --jobid tryjob-id --key os:linux"
	clTaskGoldctlImgtestInitArgs := []string{
		"imgtest",
		"init",
		"--work-dir", goldctlWorkDir,
		"--instance", "skia",
		"--url", "https://gold.skia.org",
		"--bucket", "skia-infra-gm",
		"--git_hash", "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		"--crs", "gerrit",
		"--cis", "buildbucket",
		"--changelist", "changelist-id",
		"--patchset", "1",
		"--jobid", "tryjob-id",
		"--key", "os:linux",
	}
	test("CL task, ZIP file", true /* =zip */, clTaskArgs, goldctlWorkDir, clTaskGoldctlImgtestInitStep, clTaskGoldctlImgtestInitArgs)
	test("CL task, directory", false /* =zip */, clTaskArgs, goldctlWorkDir, clTaskGoldctlImgtestInitStep, clTaskGoldctlImgtestInitArgs)
}
