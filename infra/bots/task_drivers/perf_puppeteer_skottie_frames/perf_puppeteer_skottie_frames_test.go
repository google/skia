// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"io/ioutil"
	"math/rand"
	"os"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/td"
)

func TestSetup_NPMInitializedBenchmarkOutCreated(t *testing.T) {
	benchmarkPath, err := ioutil.TempDir("", "benchmark")
	require.NoError(t, err)

	const fakeNodeBinPath = "/fake/path/to/node/bin"

	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		mock := exec.CommandCollector{}
		ctx = td.WithExecRunFn(ctx, mock.Run)
		err := setup(ctx, benchmarkPath, fakeNodeBinPath)
		if err != nil {
			assert.NoError(t, err)
			return err
		}
		require.Len(t, mock.Commands(), 1)
		cmd := mock.Commands()[0]
		assert.Equal(t, "/fake/path/to/node/bin/npm", cmd.Name)
		assert.Equal(t, []string{"ci"}, cmd.Args)
		return nil
	})
	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)

	fi, err := os.Stat(filepath.Join(benchmarkPath, "out"))
	require.NoError(t, err)
	assert.True(t, fi.IsDir())
}

func TestBenchSkottieFrames_CPUHasNoUseGPUFlag(t *testing.T) {
	lotties, err := ioutil.TempDir("", "lotties")
	require.NoError(t, err)

	require.NoError(t, os.MkdirAll(filepath.Join(lotties, "animation_1"), 0777))

	const fakeNodeBinPath = "/fake/path/to/node/bin"
	const fakeCanvasKitPath = "/fake/path/to/canvaskit"
	const fakeBenchmarkPath = "/fake/path/to/perf-puppeteer"

	perfObj := perfJSONFormat{
		Key: map[string]string{
			perfKeyCpuOrGPU: "CPU",
		},
	}

	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		mock := exec.CommandCollector{}
		ctx = td.WithExecRunFn(ctx, mock.Run)
		err := benchSkottieFrames(ctx, perfObj, fakeBenchmarkPath, fakeCanvasKitPath, lotties, fakeNodeBinPath)
		if err != nil {
			assert.NoError(t, err)
			return err
		}
		require.Len(t, mock.Commands(), 1)
		cmd := mock.Commands()[0]
		assert.Equal(t, "/fake/path/to/node/bin/node", cmd.Name)
		assert.Equal(t, []string{"perf-canvaskit-with-puppeteer",
			"--bench_html", "skottie-frames.html",
			"--canvaskit_js", "/fake/path/to/canvaskit/canvaskit.js",
			"--canvaskit_wasm", "/fake/path/to/canvaskit/canvaskit.wasm",
			"--input_lottie", filepath.Join(lotties, "animation_1", "data.json"),
			"--assets", filepath.Join(lotties, "animation_1", "images"),
			"--output", "/fake/path/to/perf-puppeteer/out/animation_1.json"}, cmd.Args)
		return nil
	})
	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)
}

func TestBenchSkottieFrames_GPUHasFlag(t *testing.T) {
	lotties, err := ioutil.TempDir("", "lotties")
	require.NoError(t, err)

	require.NoError(t, os.MkdirAll(filepath.Join(lotties, "animation_1"), 0777))

	const fakeNodeBinPath = "/fake/path/to/node/bin"
	const fakeCanvasKitPath = "/fake/path/to/canvaskit"
	const fakeBenchmarkPath = "/fake/path/to/perf-puppeteer"

	perfObj := perfJSONFormat{
		Key: map[string]string{
			perfKeyCpuOrGPU: "GPU",
		},
	}

	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		mock := exec.CommandCollector{}
		ctx = td.WithExecRunFn(ctx, mock.Run)
		err := benchSkottieFrames(ctx, perfObj, fakeBenchmarkPath, fakeCanvasKitPath, lotties, fakeNodeBinPath)
		if err != nil {
			assert.NoError(t, err)
			return err
		}
		require.Len(t, mock.Commands(), 1)
		cmd := mock.Commands()[0]
		assert.Equal(t, "/fake/path/to/node/bin/node", cmd.Name)
		assert.Equal(t, []string{"perf-canvaskit-with-puppeteer",
			"--bench_html", "skottie-frames.html",
			"--canvaskit_js", "/fake/path/to/canvaskit/canvaskit.js",
			"--canvaskit_wasm", "/fake/path/to/canvaskit/canvaskit.wasm",
			"--input_lottie", filepath.Join(lotties, "animation_1", "data.json"),
			"--assets", filepath.Join(lotties, "animation_1", "images"),
			"--output", "/fake/path/to/perf-puppeteer/out/animation_1.json",
			"--use_gpu"}, cmd.Args)
		return nil
	})
	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)
}

// TestProcessSkottieFramesData_CPUTwoInputsGetSummarizedAndCombined tests the scenario where we
// have multiple inputs to process. The input directory should get scanned for all json files;
// these JSON files should be read in and converted to perf results, using the name of the file
// as the name (w/o the .json suffix).
func TestProcessSkottieFramesData_CPUTwoInputsGetSummarizedAndCombined(t *testing.T) {
	input, err := ioutil.TempDir("", "inputs")
	require.NoError(t, err)
	err = writeFilesToDisk(filepath.Join(input, "out"), map[string]string{
		"first_animation.json":  skottieFramesSampleOne,
		"second_animation.json": skottieFramesSampleTwo,
	})
	require.NoError(t, err)
	output, err := ioutil.TempDir("", "perf")
	require.NoError(t, err)

	keys := map[string]string{
		"os":               "Debian10",
		"model":            "GCE",
		perfKeyCpuOrGPU:    "CPU",
		"cpu_or_gpu_value": "AVX2",
	}

	perfObj, err := makePerfObj(someGitHash, someTaskID, someMachineID, keys)
	require.NoError(t, err)

	outputFile := filepath.Join(output, "perf-taskid1352.json")
	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		return processSkottieFramesData(ctx, perfObj, input, outputFile)
	})
	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)

	b, err := ioutil.ReadFile(outputFile)
	require.NoError(t, err)

	assert.Equal(t, `{
  "gitHash": "032631e490db494128e0610a19adce4cab9706d1",
  "swarming_task_id": "4bdd43ed7c906c11",
  "swarming_machine_id": "skia-e-gce-203",
  "key": {
    "arch": "wasm",
    "binary": "CanvasKit",
    "browser": "Chromium",
    "configuration": "Release",
    "cpu_or_gpu": "CPU",
    "cpu_or_gpu_value": "AVX2",
    "extra_config": "SkottieFrames",
    "model": "GCE",
    "os": "Debian10"
  },
  "results": {
    "first_animation": {
      "software": {
        "1st_frame_to_flush_ms": 31.555,
        "2nd_frame_to_flush_ms": 87.795,
        "3rd_frame_to_flush_ms": 0.43,
        "4th_frame_to_flush_ms": 1.845,
        "5th_frame_to_flush_ms": 3.61,
        "90th_percentile_frame_to_flush_ms": 4.455,
        "95th_percentile_frame_to_flush_ms": 31.555,
        "99th_percentile_frame_to_flush_ms": 87.795,
        "avg_first_five_frames_ms": 25.047,
        "avg_frame_to_flush_ms": 5.662692,
        "avg_seek_ms": 0.740625,
        "json_load_ms": 16.05,
        "median_frame_to_flush_ms": 0.795,
        "median_seek_ms": 0.155,
        "stddev_frame_to_flush_ms": 17.463467,
        "stddev_seek_ms": 2.2108452
      }
    },
    "second_animation": {
      "software": {
        "1st_frame_to_flush_ms": 210.555,
        "2nd_frame_to_flush_ms": 770.795,
        "3rd_frame_to_flush_ms": 10.43,
        "4th_frame_to_flush_ms": 31.845,
        "5th_frame_to_flush_ms": 3.61,
        "90th_percentile_frame_to_flush_ms": 210.555,
        "95th_percentile_frame_to_flush_ms": 400.455,
        "99th_percentile_frame_to_flush_ms": 770.795,
        "avg_first_five_frames_ms": 205.44699,
        "avg_frame_to_flush_ms": 55.58577,
        "avg_seek_ms": 0.740625,
        "json_load_ms": 28.15,
        "median_frame_to_flush_ms": 0.8,
        "median_seek_ms": 0.155,
        "stddev_frame_to_flush_ms": 166.36926,
        "stddev_seek_ms": 2.2108452
      }
    }
  }
}`, string(b))
}

func TestProcessSkottieFramesData_GPUTwoInputsGetSummarizedAndCombined(t *testing.T) {
	input, err := ioutil.TempDir("", "inputs")
	require.NoError(t, err)
	err = writeFilesToDisk(filepath.Join(input, "out"), map[string]string{
		"first_animation.json":  skottieFramesSampleOne,
		"second_animation.json": skottieFramesSampleTwo,
	})
	require.NoError(t, err)
	output, err := ioutil.TempDir("", "perf")
	require.NoError(t, err)

	// These are based off of realistic values.
	keys := map[string]string{
		"os":               "Ubuntu18",
		"model":            "Golo",
		perfKeyCpuOrGPU:    "GPU",
		"cpu_or_gpu_value": "QuadroP400",
	}

	perfObj, err := makePerfObj(someGitHash, someTaskID, someMachineID, keys)
	require.NoError(t, err)

	outputFile := filepath.Join(output, "perf-taskid1352.json")
	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		return processSkottieFramesData(ctx, perfObj, input, outputFile)
	})
	require.Empty(t, res.Errors)

	// Re-seed the RNG, so we can get the filename the code wrote to.
	rand.Seed(0)
	b, err := ioutil.ReadFile(outputFile)
	require.NoError(t, err)

	assert.Equal(t, `{
  "gitHash": "032631e490db494128e0610a19adce4cab9706d1",
  "swarming_task_id": "4bdd43ed7c906c11",
  "swarming_machine_id": "skia-e-gce-203",
  "key": {
    "arch": "wasm",
    "binary": "CanvasKit",
    "browser": "Chromium",
    "configuration": "Release",
    "cpu_or_gpu": "GPU",
    "cpu_or_gpu_value": "QuadroP400",
    "extra_config": "SkottieFrames",
    "model": "Golo",
    "os": "Ubuntu18"
  },
  "results": {
    "first_animation": {
      "webgl2": {
        "1st_frame_to_flush_ms": 31.555,
        "2nd_frame_to_flush_ms": 87.795,
        "3rd_frame_to_flush_ms": 0.43,
        "4th_frame_to_flush_ms": 1.845,
        "5th_frame_to_flush_ms": 3.61,
        "90th_percentile_frame_to_flush_ms": 4.455,
        "95th_percentile_frame_to_flush_ms": 31.555,
        "99th_percentile_frame_to_flush_ms": 87.795,
        "avg_first_five_frames_ms": 25.047,
        "avg_frame_to_flush_ms": 5.662692,
        "avg_seek_ms": 0.740625,
        "json_load_ms": 16.05,
        "median_frame_to_flush_ms": 0.795,
        "median_seek_ms": 0.155,
        "stddev_frame_to_flush_ms": 17.463467,
        "stddev_seek_ms": 2.2108452
      }
    },
    "second_animation": {
      "webgl2": {
        "1st_frame_to_flush_ms": 210.555,
        "2nd_frame_to_flush_ms": 770.795,
        "3rd_frame_to_flush_ms": 10.43,
        "4th_frame_to_flush_ms": 31.845,
        "5th_frame_to_flush_ms": 3.61,
        "90th_percentile_frame_to_flush_ms": 210.555,
        "95th_percentile_frame_to_flush_ms": 400.455,
        "99th_percentile_frame_to_flush_ms": 770.795,
        "avg_first_five_frames_ms": 205.44699,
        "avg_frame_to_flush_ms": 55.58577,
        "avg_seek_ms": 0.740625,
        "json_load_ms": 28.15,
        "median_frame_to_flush_ms": 0.8,
        "median_seek_ms": 0.155,
        "stddev_frame_to_flush_ms": 166.36926,
        "stddev_seek_ms": 2.2108452
      }
    }
  }
}`, string(b))
}

func writeFilesToDisk(path string, fileNamesToContent map[string]string) error {
	if err := os.MkdirAll(path, 0777); err != nil {
		return err
	}
	for name, content := range fileNamesToContent {
		if err := ioutil.WriteFile(filepath.Join(path, name), []byte(content), 0666); err != nil {
			return err
		}
	}
	return nil
}

const (
	someGitHash   = "032631e490db494128e0610a19adce4cab9706d1"
	someTaskID    = "4bdd43ed7c906c11"
	someMachineID = "skia-e-gce-203"
)

const skottieFramesSampleOne = `
{
  "frames_ms": [
    31.555,
    87.795,
    0.430,
    1.845,
    3.610,
    1.105,
    0.545,
    2.315,
    1.685,
    0.615,
    0.425,
    0.815,
    0.355,
    0.655,
    0.390,
    4.455,
    0.800,
    0.685,
    2.630,
    0.325,
    0.355,
    0.740,
    0.785,
    0.795,
    0.72,
    0.80
  ],
  "seeks_ms": [
    0.5,
    0.08,
    0.07,
    0.155,
    0.29,
    0.32,
    0.185,
    0.32,
    9.29,
    0.18,
    0.075,
    0.07,
    0.06,
    0.08,
    0.065,
    0.11
  ],
  "json_load_ms": 16.05
}`

const skottieFramesSampleTwo = `
{
  "frames_ms": [
    210.555,
    770.795,
    10.430,
    31.845,
    3.610,
    1.105,
    0.545,
    2.315,
    1.685,
    0.615,
    0.425,
    0.815,
    0.355,
    0.655,
    0.390,
    400.455,
    0.800,
    0.685,
    2.630,
    0.325,
    0.355,
    0.740,
    0.785,
    0.795,
    0.72,
    0.80
  ],
  "seeks_ms": [
    0.5,
    0.08,
    0.07,
    0.155,
    0.29,
    0.32,
    0.185,
    0.32,
    9.29,
    0.18,
    0.075,
    0.07,
    0.06,
    0.08,
    0.065,
    0.11
  ],
  "json_load_ms": 28.15
}`
