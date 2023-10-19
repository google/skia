// Copyright 2023 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package common

import (
	"context"
	"path/filepath"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/gcs"
	"go.skia.org/infra/go/gcs/mocks"
	"go.skia.org/infra/go/now"

	infra_testutils "go.skia.org/infra/go/testutils"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
	"go.skia.org/skia/infra/bots/task_drivers/testutils"
)

func TestComputeBenchmarkTestRunnerCLIFlags_Success(t *testing.T) {
	test := func(name string, benchmarkInfo BenchmarkInfo, expectedFlags []string) {
		t.Run(name, func(t *testing.T) {
			ctx := WithGoldAndPerfKeyValuePairsContext(context.Background(), map[string]string{
				"os":   "linux",
				"arch": "x86_64",
			})

			actualFlags := ComputeBenchmarkTestRunnerCLIFlags(ctx, benchmarkInfo)
			assert.Equal(t, expectedFlags, actualFlags)
		})
	}

	test("post-submit task", BenchmarkInfo{
		GitCommit: "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:  "BazelTest-Foo-Bar",
		TaskID:    "1234567890",
	}, []string{
		"--key",
		"arch", "x86_64",
		"os", "linux",
		"--gitHash", "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		"--links",
		"task", "https://task-scheduler.skia.org/task/1234567890",
	})

	test("CL task", BenchmarkInfo{
		GitCommit:     "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:      "BazelTest-Foo-Bar",
		TaskID:        "1234567890",
		ChangelistID:  "12345",
		PatchsetOrder: "3",
	}, []string{
		"--key",
		"arch", "x86_64",
		"os", "linux",
		"--gitHash", "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		"--issue", "12345",
		"--patchset", "3",
		"--links",
		"task", "https://task-scheduler.skia.org/task/1234567890",
		"changelist", "https://skia-review.googlesource.com/c/skia/+/12345/3",
	})
}

func TestUploadToPerf_NoOutputsZIPOrDir_Error(t *testing.T) {
	test := func(name string, benchmarkInfo BenchmarkInfo) {
		t.Run(name, func(t *testing.T) {
			gcsClient := mocks.NewGCSClient(t)
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				err := UploadToPerf(ctx, gcsClient, benchmarkInfo, "/no/such/outputs.zip")
				assert.Error(t, err)
				assert.Contains(t, err.Error(), "stat /no/such/outputs.zip: no such file or directory")
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)
			testutils.AssertStepNames(t, res) // No steps.

			gcsClient.AssertNotCalled(t, "Bucket")
			gcsClient.AssertNotCalled(t, "SetFileContents")
			gcsClient.AssertExpectations(t)
		})
	}

	test("post-submit task", BenchmarkInfo{
		GitCommit: "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:  "BazelTest-Foo-Bar",
		TaskID:    "1234567890",
	})

	test("CL task", BenchmarkInfo{
		GitCommit:     "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:      "BazelTest-Foo-Bar",
		TaskID:        "1234567890",
		ChangelistID:  "12345",
		PatchsetOrder: "3",
	})
}

func TestUploadToPerf_OutputsZip_NoResultsJSONFile_Error(t *testing.T) {
	test := func(name string, benchmarkInfo BenchmarkInfo) {
		t.Run(name, func(t *testing.T) {
			undeclaredTestOutputs := map[string]string{
				// Does not contain a results.json file. File contents do not matter for this test.
				"some-image.png":     "fake PNG",
				"some-plaintext.txt": "fake TXT",
			}

			// Write undeclared test outputs to disk.
			outputsZIP := filepath.Join(t.TempDir(), "outputs.zip")
			testutils.MakeZIP(t, outputsZIP, undeclaredTestOutputs)

			// Will be returned by the mocked os_steps.TempDir() when the task driver tries to create a
			// directory in which to extract the undeclared outputs ZIP archive.
			outputsZIPExtractionDir := t.TempDir()

			gcsClient := mocks.NewGCSClient(t)
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				// We don't need to assert the exact number of times that os_steps.TempDir() is called
				// because said function produces a "Creating TempDir" task driver step, and we check the
				// exact set of steps produced.
				ctx = context.WithValue(ctx, os_steps.TempDirContextKey, testutils.MakeTempDirMockFn(t, outputsZIPExtractionDir))

				err := UploadToPerf(ctx, gcsClient, benchmarkInfo, outputsZIP)
				assert.Error(t, err)
				assert.Contains(t, err.Error(), "stat "+outputsZIPExtractionDir+"/results.json: no such file or directory")
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			testutils.AssertStepNames(t, res,
				"Creating TempDir",
				"Extract undeclared outputs archive "+outputsZIP+" into "+outputsZIPExtractionDir,
				"Extracting file: some-image.png",
				"Not extracting non-PNG / non-JSON file: some-plaintext.txt",
				"Stat "+outputsZIPExtractionDir+"/results.json",
			)

			gcsClient.AssertNotCalled(t, "Bucket")
			gcsClient.AssertNotCalled(t, "SetFileContents")
			gcsClient.AssertExpectations(t)
		})
	}

	test("post-submit task", BenchmarkInfo{
		GitCommit: "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:  "BazelTest-Foo-Bar",
		TaskID:    "1234567890",
	})

	test("CL task", BenchmarkInfo{
		GitCommit:     "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:      "BazelTest-Foo-Bar",
		TaskID:        "1234567890",
		ChangelistID:  "12345",
		PatchsetOrder: "3",
	})
}

func TestUploadToPerf_OutputsDirectory_NoResultsJSONFile_Error(t *testing.T) {
	test := func(name string, benchmarkInfo BenchmarkInfo) {
		t.Run(name, func(t *testing.T) {
			undeclaredTestOutputs := map[string]string{
				// Does not contain a results.json file. File contents do not matter for this test.
				"some-image.png":     "fake PNG",
				"some-plaintext.txt": "fake TXT",
			}

			// Write undeclared test outputs to disk.
			outputsDir := t.TempDir()
			testutils.PopulateDir(t, outputsDir, undeclaredTestOutputs)

			gcsClient := mocks.NewGCSClient(t)
			res := td.RunTestSteps(t, false, func(ctx context.Context) error {
				err := UploadToPerf(ctx, gcsClient, benchmarkInfo, outputsDir)
				assert.Error(t, err)
				assert.Contains(t, err.Error(), "stat "+outputsDir+"/results.json: no such file or directory")
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			testutils.AssertStepNames(t, res, "Stat "+outputsDir+"/results.json")

			gcsClient.AssertNotCalled(t, "Bucket")
			gcsClient.AssertNotCalled(t, "SetFileContents")
			gcsClient.AssertExpectations(t)
		})
	}

	test("post-submit task", BenchmarkInfo{
		GitCommit: "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:  "BazelTest-Foo-Bar",
		TaskID:    "1234567890",
	})

	test("CL task", BenchmarkInfo{
		GitCommit:     "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:      "BazelTest-Foo-Bar",
		TaskID:        "1234567890",
		ChangelistID:  "12345",
		PatchsetOrder: "3",
	})
}

func TestUploadToPerf_OutputsZip_Success(t *testing.T) {
	test := func(name string, benchmarkInfo BenchmarkInfo) {
		t.Run(name, func(t *testing.T) {
			resultsJSONFileContents := `{"foo": "this test requires that this file exists; its contents do not matter"}`
			undeclaredTestOutputs := map[string]string{
				"results.json":       resultsJSONFileContents,
				"some-image.png":     "fake PNG",
				"some-plaintext.txt": "fake TXT",
			}

			// Write undeclared test outputs to disk.
			outputsZIP := filepath.Join(t.TempDir(), "outputs.zip")
			testutils.MakeZIP(t, outputsZIP, undeclaredTestOutputs)

			// Will be returned by the mocked os_steps.TempDir() when the task driver tries to create a
			// directory in which to extract the undeclared outputs ZIP archive.
			outputsZIPExtractionDir := t.TempDir()

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

				// We don't need to assert the exact number of times that os_steps.TempDir() is called
				// because said function produces a "Creating TempDir" task driver step, and we check the
				// exact set of steps produced.
				ctx = context.WithValue(ctx, os_steps.TempDirContextKey, testutils.MakeTempDirMockFn(t, outputsZIPExtractionDir))

				err := UploadToPerf(ctx, gcsClient, benchmarkInfo, outputsZIP)
				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			testutils.AssertStepNames(t, res,
				"Creating TempDir",
				"Extract undeclared outputs archive "+outputsZIP+" into "+outputsZIPExtractionDir,
				"Extracting file: results.json",
				"Extracting file: some-image.png",
				"Not extracting non-PNG / non-JSON file: some-plaintext.txt",
				"Stat "+outputsZIPExtractionDir+"/results.json",
				"Read "+outputsZIPExtractionDir+"/results.json",
				"Upload gs://skia-perf/nano-json-v1/2022/01/31/01/ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99/BazelTest-Foo-Bar/results_1234567890.json",
			)

			gcsClient.AssertExpectations(t)
		})
	}

	test("post-submit task", BenchmarkInfo{
		GitCommit: "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:  "BazelTest-Foo-Bar",
		TaskID:    "1234567890",
	})

	test("CL task", BenchmarkInfo{
		GitCommit:     "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:      "BazelTest-Foo-Bar",
		TaskID:        "1234567890",
		ChangelistID:  "12345",
		PatchsetOrder: "3",
	})
}

func TestUploadToPerf_OutputsDirectory_Success(t *testing.T) {
	test := func(name string, benchmarkInfo BenchmarkInfo) {
		t.Run(name, func(t *testing.T) {
			resultsJSONFileContents := `{"foo": "this test requires that this file exists; its contents do not matter"}`
			undeclaredTestOutputs := map[string]string{
				"results.json":       resultsJSONFileContents,
				"some-image.png":     "fake PNG",
				"some-plaintext.txt": "fake TXT",
			}

			// Write undeclared test outputs to disk.
			outputsDir := t.TempDir()
			testutils.PopulateDir(t, outputsDir, undeclaredTestOutputs)

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

				err := UploadToPerf(ctx, gcsClient, benchmarkInfo, outputsDir)
				assert.NoError(t, err)
				return err
			})

			require.Empty(t, res.Errors)
			require.Empty(t, res.Exceptions)

			testutils.AssertStepNames(t, res,
				"Stat "+outputsDir+"/results.json",
				"Read "+outputsDir+"/results.json",
				"Upload gs://skia-perf/nano-json-v1/2022/01/31/01/ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99/BazelTest-Foo-Bar/results_1234567890.json",
			)

			gcsClient.AssertExpectations(t)
		})
	}

	test("post-submit task, directory", BenchmarkInfo{
		GitCommit: "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:  "BazelTest-Foo-Bar",
		TaskID:    "1234567890",
	})

	test("CL task, directory", BenchmarkInfo{
		GitCommit:     "ff99ff99ff99ff99ff99ff99ff99ff99ff99ff99",
		TaskName:      "BazelTest-Foo-Bar",
		TaskID:        "1234567890",
		ChangelistID:  "12345",
		PatchsetOrder: "3",
	})
}
