// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"os"
	"path/filepath"
	"regexp"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/common"
	"go.skia.org/skia/infra/bots/task_drivers/testutils"
)

func TestRun_Success(t *testing.T) {
	// Given that we have tests for common.UploadToGold(), it suffices to test a couple of
	// "happy" cases here.

	test := func(name string, tdArgs taskDriverArgs, goldctlImgtestInitStepName *regexp.Regexp, goldctlImgtestInitArgsStringsOrRegexps []any) {
		t.Run(name, func(t *testing.T) {
			// Create fake archive with undeclared test outputs.
			outputsZipPath := filepath.Join(tdArgs.checkoutDir, "bazel-testlogs", "some", "test", "target", "test.outputs", "outputs.zip")
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
			})

			commandCollector := exec.CommandCollector{}
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				ctx = td.WithExecRunFn(ctx, commandCollector.Run)

				err := run(ctx, tdArgs)

				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			testutils.AssertStepNamesMatchStringsOrRegexps(t, res,
				"Test //some/test:target with config linux_rbe",
				"bazelisk test //some/test:target --config=linux_rbe --test_output=errors --jobs=100",
				regexp.MustCompile("^Extract undeclared outputs archive .+/bazel-testlogs/some/test/target/test.outputs/outputs.zip into .+$"),
				"Extracting file: alfa.json",
				"Extracting file: alfa.png",
				"Extracting file: beta.json",
				"Extracting file: beta.png",
				"Gather JSON and PNG files produced by GMs",
				"Gather \"alfa.png\"",
				"Gather \"beta.png\"",
				"Upload GM outputs to Gold",
				"Creating TempDir",
				regexp.MustCompile("^/path/to/goldctl auth --work-dir [a-zA-Z0-9-/]+ --luci$"),
				goldctlImgtestInitStepName,
				regexp.MustCompile("^/path/to/goldctl imgtest add --work-dir [a-zA-Z0-9-/]+ --test-name alfa --png-file [a-zA-Z0-9-/]+/alfa.png --png-digest a01a01a01a01a01a01a01a01a01a01a0 --add-test-key build_system:bazel --add-test-key name:alfa --add-test-key source_type:gm$"),
				regexp.MustCompile("^/path/to/goldctl imgtest add --work-dir [a-zA-Z0-9-/]+ --test-name beta --png-file [a-zA-Z0-9-/]+/beta.png --png-digest b02b02b02b02b02b02b02b02b02b02b0 --add-test-key build_system:bazel --add-test-key name:beta --add-test-key source_type:gm$"),
				regexp.MustCompile("^/path/to/goldctl imgtest finalize --work-dir [a-zA-Z0-9-/]+$"),
			)

			require.Len(t, commandCollector.Commands(), 6)

			testutils.AssertCommand(t, commandCollector.Commands()[0],
				tdArgs.checkoutDir,
				"bazelisk",
				"test",
				"//some/test:target",
				"--config=linux_rbe",
				"--test_output=errors",
				"--jobs=100",
			)

			testutils.AssertCommand(t, commandCollector.Commands()[1],
				testutils.IgnoreCommandDir,
				"/path/to/goldctl",
				"auth",
				"--work-dir",
				regexp.MustCompile("^[a-zA-Z0-9-/]+$"),
				"--luci",
			)

			testutils.AssertCommand(t, commandCollector.Commands()[2],
				testutils.IgnoreCommandDir,
				"/path/to/goldctl",
				goldctlImgtestInitArgsStringsOrRegexps...,
			)

			testutils.AssertCommand(t, commandCollector.Commands()[3],
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

			testutils.AssertCommand(t, commandCollector.Commands()[4],
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

			testutils.AssertCommand(t, commandCollector.Commands()[5],
				testutils.IgnoreCommandDir,
				"/path/to/goldctl",
				"imgtest",
				"finalize",
				"--work-dir",
				regexp.MustCompile("^[a-zA-Z0-9-/]+$"),
			)
		})
	}

	checkoutDir := t.TempDir()

	test(
		"post-submit task",
		taskDriverArgs{
			UploadToGoldArgs: common.UploadToGoldArgs{
				TestOnlyAllowAnyBazelLabel: true,
				BazelLabel:                 "//some/test:target",
				GoldctlPath:                "/path/to/goldctl",
				GitCommit:                  "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
			},
			checkoutDir: checkoutDir,
			bazelConfig: "linux_rbe",
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
		taskDriverArgs{
			UploadToGoldArgs: common.UploadToGoldArgs{
				TestOnlyAllowAnyBazelLabel: true,
				BazelLabel:                 "//some/test:target",
				GoldctlPath:                "/path/to/goldctl",
				GitCommit:                  "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
				ChangelistID:               "changelist-id",
				PatchsetOrder:              "1",
				TryjobID:                   "tryjob-id",
			},
			checkoutDir: checkoutDir,
			bazelConfig: "linux_rbe",
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
