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
	"sort"

	"go.skia.org/infra/go/gcs"
	"go.skia.org/infra/go/now"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

// PerfGCSBucketName is the name of the perf.skia.org GCS bucket.
const PerfGCSBucketName = "skia-perf"

// The name of the file produced by the C++ test that we should upload to Perf.
const resultsJSON = "results.json"

// BenchmarkInfo contains information about the CI task under which a benchmark is executed.
type BenchmarkInfo struct {
	GitCommit     string
	TaskName      string
	TaskID        string
	ChangelistID  string
	PatchsetOrder string
}

// ComputeBenchmarkTestRunnerCLIFlags returns the command-line flags that should be passed to the
// benchmark test runner.
func ComputeBenchmarkTestRunnerCLIFlags(ctx context.Context, benchmarkInfo BenchmarkInfo) []string {
	flags := computeBenchmarkTestRunnerKeyFlag(ctx)

	flags = append(flags,
		"--gitHash", benchmarkInfo.GitCommit)

	if benchmarkInfo.ChangelistID != "" && benchmarkInfo.PatchsetOrder != "" {
		flags = append(flags,
			"--issue",
			benchmarkInfo.ChangelistID,
			"--patchset",
			benchmarkInfo.PatchsetOrder)
	}

	flags = append(flags,
		"--links",
		"task",
		"https://task-scheduler.skia.org/task/"+benchmarkInfo.TaskID)
	if benchmarkInfo.ChangelistID != "" && benchmarkInfo.PatchsetOrder != "" {
		flags = append(flags,
			"changelist",
			fmt.Sprintf("https://skia-review.googlesource.com/c/skia/+/%s/%s", benchmarkInfo.ChangelistID, benchmarkInfo.PatchsetOrder))
	}

	return flags
}

func computeBenchmarkTestRunnerKeyFlag(ctx context.Context) []string {
	keyValuePairs := ComputeGoldAndPerfKeyValuePairs(ctx)

	// Sort keys for determinism.
	var keys []string
	for key := range keyValuePairs {
		keys = append(keys, key)
	}
	sort.Strings(keys)

	flag := []string{"--key"}
	for _, key := range keys {
		flag = append(flag, key, keyValuePairs[key])
	}

	return flag
}

func UploadToPerf(ctx context.Context, gcsClient gcs.GCSClient, benchmarkInfo BenchmarkInfo, outputsZIPOrDir string) error {
	// Fail loudly if the outputs ZIP or directory does not exist. All benchmark tests should produce
	// a results.json file.
	fileInfo, err := os.Stat(outputsZIPOrDir)
	if err != nil {
		return skerr.Wrap(err)
	}

	// If the undeclared outputs ZIP file or directory is a ZIP file, extract it.
	outputsDir := ""
	if fileInfo.IsDir() {
		outputsDir = outputsZIPOrDir
	} else {
		var err error
		outputsDir, err = ExtractOutputsZip(ctx, outputsZIPOrDir)
		if err != nil {
			return skerr.Wrap(err)
		}
		defer util.RemoveAll(outputsDir)
	}

	// Get path to the results.json file that we will upload to Perf.
	resultsJSONPath := filepath.Join(outputsDir, resultsJSON)
	fileInfo, err = os_steps.Stat(ctx, resultsJSONPath)
	if err != nil {
		return skerr.Wrapf(err, "while stating %q", resultsJSONPath)
	}
	if fileInfo.IsDir() {
		return skerr.Fmt("file %q is a directory", resultsJSONPath)
	}

	resultsJSONBytes, err := os_steps.ReadFile(ctx, resultsJSONPath)
	if err != nil {
		return skerr.Wrap(err)
	}

	gcsPath := computePerfJSONFileGCSPath(ctx, benchmarkInfo)
	gcsURL := fmt.Sprintf("gs://%s/%s", gcsClient.Bucket(), gcsPath)
	return td.Do(ctx, td.Props(fmt.Sprintf("Upload %s", gcsURL)), func(ctx context.Context) error {
		if err := gcsClient.SetFileContents(ctx, gcsPath, gcs.FILE_WRITE_OPTS_TEXT, resultsJSONBytes); err != nil {
			return skerr.Wrapf(err, "while uploading %q", gcsURL)
		}
		return nil
	})
}

// Based on
// https://skia.googlesource.com/skia/+/5096ed4deb06171700c74273b685713bec0ae597/infra/bots/task_drivers/codesize/codesize.go#504
// and
// https://skia.googlesource.com/skia/+/5096ed4deb06171700c74273b685713bec0ae597/infra/bots/task_drivers/codesize/codesize.go#517.
func computePerfJSONFileGCSPath(ctx context.Context, benchmarkInfo BenchmarkInfo) string {
	timePrefix := now.Now(ctx).UTC().Format("2006/01/02/15") // YYYY/MM/DD/HH.
	// Example: nano-json-v1/2022/01/31/01/033ccea12c0949d0f712471bfcb4ed6daf69aaff/BazelTest-Foo-Bar/results_CkPp9ElAaEXyYWNHpXHU.json.
	return fmt.Sprintf("nano-json-v1/%s/%s/%s/results_%s.json", timePrefix, benchmarkInfo.GitCommit, benchmarkInfo.TaskName, benchmarkInfo.TaskID)
}
