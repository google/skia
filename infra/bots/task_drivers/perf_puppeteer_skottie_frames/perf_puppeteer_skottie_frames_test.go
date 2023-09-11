// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"io/ioutil"
	"os"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/testutils"
	"go.skia.org/infra/task_driver/go/td"
)

func TestSetup_NPMInitializedBenchmarkOutCreated(t *testing.T) {
	benchmarkPath, err := ioutil.TempDir("", "benchmark")
	require.NoError(t, err)
	defer testutils.RemoveAll(t, benchmarkPath)

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
	defer testutils.RemoveAll(t, lotties)

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
			"--output", "/fake/path/to/perf-puppeteer/out/animation_1.json",
			"--timeout=90"}, cmd.Args)
		return nil
	})
	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)
}

func TestBenchSkottieFrames_GPUHasFlag(t *testing.T) {
	lotties, err := ioutil.TempDir("", "lotties")
	require.NoError(t, err)
	defer testutils.RemoveAll(t, lotties)

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
	defer testutils.RemoveAll(t, input)
	err = writeFilesToDisk(filepath.Join(input, "out"), map[string]string{
		"first_animation.json":  skottieFramesSampleOne,
		"second_animation.json": skottieFramesSampleTwo,
	})
	require.NoError(t, err)
	output, err := ioutil.TempDir("", "perf")
	require.NoError(t, err)
	defer testutils.RemoveAll(t, output)

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
        "1st_frame_ms": 31.555,
        "2nd_frame_ms": 87.795,
        "3rd_frame_ms": 0.43,
        "4th_frame_ms": 1.845,
        "5th_frame_ms": 3.61,
        "90th_percentile_frame_ms": 4.455,
        "95th_percentile_frame_ms": 31.555,
        "99th_percentile_frame_ms": 87.795,
        "avg_first_five_frames_ms": 25.047,
        "avg_render_frame_ms": 5.662692,
        "avg_render_with_flush_ms": 1.75,
        "avg_render_without_flush_ms": 1.875,
        "json_load_ms": 16.05,
        "median_render_frame_ms": 0.795,
        "median_render_with_flush_ms": 1.8,
        "median_render_without_flush_ms": 1.88,
        "stddev_render_frame_ms": 17.463467,
        "stddev_render_with_flush_ms": 0.74999994,
        "stddev_render_without_flush_ms": 0.07500001
      }
    },
    "second_animation": {
      "software": {
        "1st_frame_ms": 210.555,
        "2nd_frame_ms": 770.795,
        "3rd_frame_ms": 10.43,
        "4th_frame_ms": 31.845,
        "5th_frame_ms": 3.61,
        "90th_percentile_frame_ms": 210.555,
        "95th_percentile_frame_ms": 400.455,
        "99th_percentile_frame_ms": 770.795,
        "avg_first_five_frames_ms": 205.44699,
        "avg_render_frame_ms": 55.58577,
        "avg_render_with_flush_ms": 3.75,
        "avg_render_without_flush_ms": 5.125,
        "json_load_ms": 28.15,
        "median_render_frame_ms": 0.8,
        "median_render_with_flush_ms": 3.8,
        "median_render_without_flush_ms": 5.13,
        "stddev_render_frame_ms": 166.36926,
        "stddev_render_with_flush_ms": 0.75,
        "stddev_render_without_flush_ms": 0.074999936
      }
    }
  }
}`, string(b))
}

func TestProcessSkottieFramesData_GPUTwoInputsGetSummarizedAndCombined(t *testing.T) {
	input, err := ioutil.TempDir("", "inputs")
	require.NoError(t, err)
	defer testutils.RemoveAll(t, input)
	err = writeFilesToDisk(filepath.Join(input, "out"), map[string]string{
		"first_animation.json":  skottieFramesSampleOne,
		"second_animation.json": skottieFramesSampleTwo,
	})
	require.NoError(t, err)
	output, err := ioutil.TempDir("", "perf")
	require.NoError(t, err)
	defer testutils.RemoveAll(t, output)

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
        "1st_frame_ms": 31.555,
        "2nd_frame_ms": 87.795,
        "3rd_frame_ms": 0.43,
        "4th_frame_ms": 1.845,
        "5th_frame_ms": 3.61,
        "90th_percentile_frame_ms": 4.455,
        "95th_percentile_frame_ms": 31.555,
        "99th_percentile_frame_ms": 87.795,
        "avg_first_five_frames_ms": 25.047,
        "avg_render_frame_ms": 5.662692,
        "avg_render_with_flush_ms": 1.75,
        "avg_render_without_flush_ms": 1.875,
        "json_load_ms": 16.05,
        "median_render_frame_ms": 0.795,
        "median_render_with_flush_ms": 1.8,
        "median_render_without_flush_ms": 1.88,
        "stddev_render_frame_ms": 17.463467,
        "stddev_render_with_flush_ms": 0.74999994,
        "stddev_render_without_flush_ms": 0.07500001
      }
    },
    "second_animation": {
      "webgl2": {
        "1st_frame_ms": 210.555,
        "2nd_frame_ms": 770.795,
        "3rd_frame_ms": 10.43,
        "4th_frame_ms": 31.845,
        "5th_frame_ms": 3.61,
        "90th_percentile_frame_ms": 210.555,
        "95th_percentile_frame_ms": 400.455,
        "99th_percentile_frame_ms": 770.795,
        "avg_first_five_frames_ms": 205.44699,
        "avg_render_frame_ms": 55.58577,
        "avg_render_with_flush_ms": 3.75,
        "avg_render_without_flush_ms": 5.125,
        "json_load_ms": 28.15,
        "median_render_frame_ms": 0.8,
        "median_render_with_flush_ms": 3.8,
        "median_render_without_flush_ms": 5.13,
        "stddev_render_frame_ms": 166.36926,
        "stddev_render_with_flush_ms": 0.75,
        "stddev_render_without_flush_ms": 0.074999936
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
  "total_frame_ms": [
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
  "without_flush_ms": [
    2.0,
    1.99,
    1.98,
    1.97,
    1.96,
    1.95,
    1.94,
    1.93,
    1.92,
    1.91,
    1.9,
    1.89,
    1.88,
    1.87,
    1.86,
    1.85,
    1.84,
    1.83,
    1.82,
    1.81,
    1.8,
    1.79,
    1.78,
    1.77,
    1.76,
    1.75
  ],
  "with_flush_ms": [
    3.0,
    2.9,
    2.8,
    2.7,
    2.6,
    2.5,
    2.4,
    2.3,
    2.2,
    2.1,
    2.0,
    1.9,
    1.8,
    1.7,
    1.6,
    1.5,
    1.4,
    1.3,
    1.2,
    1.1,
    1.0,
    0.9,
    0.8,
    0.7,
    0.6,
    0.5
  ],
  "json_load_ms": 16.05
}`

const skottieFramesSampleTwo = `
{
  "total_frame_ms": [
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
  "without_flush_ms": [
    5.0,
    5.01,
    5.02,
    5.03,
    5.04,
    5.05,
    5.06,
    5.07,
    5.08,
    5.09,
    5.1,
    5.11,
    5.12,
    5.13,
    5.14,
    5.15,
    5.16,
    5.17,
    5.18,
    5.19,
    5.2,
    5.21,
    5.22,
    5.23,
    5.24,
    5.25
  ],
  "with_flush_ms": [
    5.0,
    4.9,
    4.8,
    4.7,
    4.6,
    4.5,
    4.4,
    4.3,
    4.2,
    4.1,
    4.0,
    3.9,
    3.8,
    3.7,
    3.6,
    3.5,
    3.4,
    3.3,
    3.2,
    3.1,
    3.0,
    2.9,
    2.8,
    2.7,
    2.6,
    2.5
  ],
  "json_load_ms": 28.15
}`
