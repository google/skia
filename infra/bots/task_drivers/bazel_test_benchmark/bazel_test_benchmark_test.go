// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/exec"
	exec_testutils "go.skia.org/infra/go/exec/testutils"
	"go.skia.org/infra/go/gcs"
	"go.skia.org/infra/go/gcs/mocks"
	"go.skia.org/infra/go/now"
	infra_testutils "go.skia.org/infra/go/testutils"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/common"
	"go.skia.org/skia/infra/bots/task_drivers/testutils"
)

func TestRun_Success(t *testing.T) {
	// Given that we have tests for common.UploadToPerf(), it suffices to test a couple of
	// "happy" cases here.

	test := func(name string, tdArgs taskDriverArgs, bazelInvocation []string) {
		t.Run(name, func(t *testing.T) {
			// Create fake archive with undeclared test outputs.
			outputsZIP := filepath.Join(tdArgs.checkoutDir, "bazel-testlogs", "some", "test", "target", "test.outputs", "outputs.zip")
			require.NoError(t, os.MkdirAll(filepath.Dir(outputsZIP), 0700))
			resultsJSONFileContents := `{"foo": "this test requires that this file exists; its contents do not matter"}`
			testutils.MakeZIP(t, outputsZIP, map[string]string{
				"results.json":       resultsJSONFileContents,
				"some-image.png":     "fake PNG",
				"some-plaintext.txt": "fake TXT",
			})

			// Will be returned by the mocked os_steps.TempDir() when the task driver tries to create a
			// directory in which to extract the undeclared outputs ZIP archive.
			outputsZIPExtractionDir := t.TempDir()

			commandCollector := exec.CommandCollector{}

			gcsClient := mocks.NewGCSClient(t)
			gcsClient.On("Bucket").Return("skia-perf")
			gcsClient.On(
				"SetFileContents",
				infra_testutils.AnyContext,
				"nano-json-v1/2022/01/31/01/ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99/BazelTest-Foo-Bar/results_1234567890.json",
				gcs.FILE_WRITE_OPTS_TEXT,
				[]byte(resultsJSONFileContents)).
				Return(nil).Once()

			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				// Make sure we use UTC instead of the system timezone. The GCS path reflects the fact that
				// we convert from UTC+1 to UTC.
				fakeNow := time.Date(2022, time.January, 31, 2, 2, 3, 0, time.FixedZone("UTC+1", 60*60))
				ctx = now.TimeTravelingContext(fakeNow).WithContext(ctx)

				ctx = common.WithGoldAndPerfKeyValuePairsContext(ctx, map[string]string{
					"os":   "linux",
					"arch": "x86_64",
				})

				ctx = td.WithExecRunFn(ctx, commandCollector.Run)
				var bazelCacheDirPath string
				ctx, bazelCacheDirPath = common.WithEnoughSpaceOnBazelCachePartitionTestOnlyContext(ctx)

				// We don't need to assert the exact number of times that os_steps.TempDir() is called
				// because said function produces a "Creating TempDir" task driver step, and we check the
				// exact set of steps produced.
				ctx = context.WithValue(ctx, os_steps.TempDirContextKey, testutils.MakeTempDirMockFn(t, outputsZIPExtractionDir))

				err := run(ctx, bazelCacheDirPath, tdArgs, gcsClient)

				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			testutils.AssertStepNames(t, res,
				"Test //some/test:target with config linux_rbe",
				strings.Join(bazelInvocation, " "),
				"Creating TempDir",
				"Extract undeclared outputs archive "+outputsZIP+" into "+outputsZIPExtractionDir,
				"Extracting file: results.json",
				"Extracting file: some-image.png",
				"Not extracting non-PNG / non-JSON file: some-plaintext.txt",
				"Stat "+outputsZIPExtractionDir+"/results.json",
				"Read "+outputsZIPExtractionDir+"/results.json",
				"Upload gs://skia-perf/nano-json-v1/2022/01/31/01/ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99/BazelTest-Foo-Bar/results_1234567890.json",
				"Clean Bazel cache if disk space is too low",
				"No need to clear the Bazel cache: free space on partition /mnt/pd0 is 20000000000 bytes, which is above the threshold of 15000000000 bytes",
			)

			// Command "bazelisk test ..." should be called from the checkout directory.
			assert.Equal(t, tdArgs.checkoutDir, commandCollector.Commands()[0].Dir)
			exec_testutils.AssertCommandsMatch(t, [][]string{bazelInvocation}, commandCollector.Commands())

			gcsClient.AssertExpectations(t)
		})
	}

	checkoutDir := t.TempDir()

	test(
		"post-submit task",
		taskDriverArgs{
			BenchmarkInfo: common.BenchmarkInfo{
				GitCommit: "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
				TaskName:  "BazelTest-Foo-Bar",
				TaskID:    "1234567890",
			},
			checkoutDir: checkoutDir,
			bazelLabel:  "//some/test:target",
			bazelConfig: "linux_rbe",
		},
		[]string{
			"bazelisk",
			"test",
			"//some/test:target",
			"--config=linux_rbe",
			"--test_output=errors",
			"--jobs=100",
			"--strategy=TestRunner=local",
			"--test_arg=--key",
			"--test_arg=arch",
			"--test_arg=x86_64",
			"--test_arg=os",
			"--test_arg=linux",
			"--test_arg=--gitHash",
			"--test_arg=ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
			"--test_arg=--links",
			"--test_arg=task",
			"--test_arg=https://task-scheduler.skia.org/task/1234567890",
		})

	test(
		"CL task",
		taskDriverArgs{
			BenchmarkInfo: common.BenchmarkInfo{
				GitCommit:     "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
				TaskName:      "BazelTest-Foo-Bar",
				TaskID:        "1234567890",
				ChangelistID:  "12345",
				PatchsetOrder: "3",
			},
			checkoutDir: checkoutDir,
			bazelLabel:  "//some/test:target",
			bazelConfig: "linux_rbe",
		},
		[]string{
			"bazelisk",
			"test",
			"//some/test:target",
			"--config=linux_rbe",
			"--test_output=errors",
			"--jobs=100",
			"--strategy=TestRunner=local",
			"--test_arg=--key",
			"--test_arg=arch",
			"--test_arg=x86_64",
			"--test_arg=os",
			"--test_arg=linux",
			"--test_arg=--gitHash",
			"--test_arg=ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
			"--test_arg=--issue",
			"--test_arg=12345",
			"--test_arg=--patchset",
			"--test_arg=3",
			"--test_arg=--links",
			"--test_arg=task",
			"--test_arg=https://task-scheduler.skia.org/task/1234567890",
			"--test_arg=changelist",
			"--test_arg=https://skia-review.googlesource.com/c/skia/+/12345/3",
		})
}
