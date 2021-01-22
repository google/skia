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

func TestSetup_NPMInitializedChromeStoppedBenchmarkOutCreated(t *testing.T) {
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
		cmds := mock.Commands()
		require.Len(t, cmds, 2)
		cmd := cmds[0]
		assert.Equal(t, "/fake/path/to/node/bin/npm", cmd.Name)
		assert.Equal(t, []string{"ci"}, cmd.Args)

		cmd = cmds[1]
		assert.Equal(t, "killall", cmd.Name)
		assert.Equal(t, []string{"chrome"}, cmd.Args)
		return nil
	})
	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)

	fi, err := os.Stat(filepath.Join(benchmarkPath, "out"))
	require.NoError(t, err)
	assert.True(t, fi.IsDir())
}

func TestBenchSKPs_CPUHasNoUseGPUFlag(t *testing.T) {
	skps, err := ioutil.TempDir("", "skps")
	require.NoError(t, err)
	defer testutils.RemoveAll(t, skps)

	require.NoError(t, ioutil.WriteFile(filepath.Join(skps, "first_skp"), []byte("doesnt matter"), 0777))

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
		err := benchSKPs(ctx, perfObj, fakeBenchmarkPath, fakeCanvasKitPath, skps, fakeNodeBinPath)
		if err != nil {
			assert.NoError(t, err)
			return err
		}
		require.Len(t, mock.Commands(), 1)
		cmd := mock.Commands()[0]
		assert.Equal(t, "/fake/path/to/node/bin/node", cmd.Name)
		assert.Equal(t, []string{"perf-canvaskit-with-puppeteer",
			"--bench_html", "render-skp.html",
			"--canvaskit_js", "/fake/path/to/canvaskit/canvaskit.js",
			"--canvaskit_wasm", "/fake/path/to/canvaskit/canvaskit.wasm",
			"--timeout", "90",
			"--input_skp", filepath.Join(skps, "first_skp"),
			"--output", "/fake/path/to/perf-puppeteer/out/first_skp.json"}, cmd.Args)
		return nil
	})
	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)
}

func TestBenchSKPs_SkiplistIsUsed(t *testing.T) {
	skps, err := ioutil.TempDir("", "skps")
	require.NoError(t, err)
	defer testutils.RemoveAll(t, skps)

	require.NoError(t, ioutil.WriteFile(filepath.Join(skps, "desk_carsvg.skp"), []byte("doesnt matter"), 0777))

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
		err := benchSKPs(ctx, perfObj, fakeBenchmarkPath, fakeCanvasKitPath, skps, fakeNodeBinPath)
		if err != nil {
			assert.NoError(t, err)
			return err
		}
		// Should be skipped
		require.Len(t, mock.Commands(), 0)
		return nil
	})
	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)
}

func TestBenchSKPs_GPUHasFlag(t *testing.T) {
	skps, err := ioutil.TempDir("", "skps")
	require.NoError(t, err)
	defer testutils.RemoveAll(t, skps)

	require.NoError(t, ioutil.WriteFile(filepath.Join(skps, "first_skp"), []byte("doesnt matter"), 0777))

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
		err := benchSKPs(ctx, perfObj, fakeBenchmarkPath, fakeCanvasKitPath, skps, fakeNodeBinPath)
		if err != nil {
			assert.NoError(t, err)
			return err
		}
		require.Len(t, mock.Commands(), 1)
		cmd := mock.Commands()[0]
		assert.Equal(t, "/fake/path/to/node/bin/node", cmd.Name)
		assert.Equal(t, []string{"perf-canvaskit-with-puppeteer",
			"--bench_html", "render-skp.html",
			"--canvaskit_js", "/fake/path/to/canvaskit/canvaskit.js",
			"--canvaskit_wasm", "/fake/path/to/canvaskit/canvaskit.wasm",
			"--timeout", "90",
			"--input_skp", filepath.Join(skps, "first_skp"),
			"--output", "/fake/path/to/perf-puppeteer/out/first_skp.json",
			"--use_gpu"}, cmd.Args)
		return nil
	})
	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)
}

func TestBenchSKPs_WebGL1(t *testing.T) {
	skps, err := ioutil.TempDir("", "skps")
	require.NoError(t, err)
	defer testutils.RemoveAll(t, skps)

	require.NoError(t, ioutil.WriteFile(filepath.Join(skps, "first_skp"), []byte("doesnt matter"), 0777))

	const fakeNodeBinPath = "/fake/path/to/node/bin"
	const fakeCanvasKitPath = "/fake/path/to/canvaskit"
	const fakeBenchmarkPath = "/fake/path/to/perf-puppeteer"

	perfObj := perfJSONFormat{
		Key: map[string]string{
			perfKeyCpuOrGPU:     "GPU",
			perfKeyWebGLVersion: "1",
		},
	}

	res := td.RunTestSteps(t, false, func(ctx context.Context) error {
		mock := exec.CommandCollector{}
		ctx = td.WithExecRunFn(ctx, mock.Run)
		err := benchSKPs(ctx, perfObj, fakeBenchmarkPath, fakeCanvasKitPath, skps, fakeNodeBinPath)
		if err != nil {
			assert.NoError(t, err)
			return err
		}
		require.Len(t, mock.Commands(), 1)
		cmd := mock.Commands()[0]
		assert.Equal(t, "/fake/path/to/node/bin/node", cmd.Name)
		assert.Equal(t, []string{"perf-canvaskit-with-puppeteer",
			"--bench_html", "render-skp.html",
			"--canvaskit_js", "/fake/path/to/canvaskit/canvaskit.js",
			"--canvaskit_wasm", "/fake/path/to/canvaskit/canvaskit.wasm",
			"--timeout", "90",
			"--input_skp", filepath.Join(skps, "first_skp"),
			"--output", "/fake/path/to/perf-puppeteer/out/first_skp.json",
			"--use_gpu",
			"--query_params webgl1"}, cmd.Args)
		return nil
	})
	require.Empty(t, res.Errors)
	require.Empty(t, res.Exceptions)
}

func TestProcessSkottieFramesData_GPUTwoInputsGetSummarizedAndCombined(t *testing.T) {
	input, err := ioutil.TempDir("", "inputs")
	require.NoError(t, err)
	defer testutils.RemoveAll(t, input)
	err = writeFilesToDisk(filepath.Join(input, "out"), map[string]string{
		"first_skp.json":  firstSKP,
		"second_skp.json": secondSKP,
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
		return processSKPData(ctx, perfObj, input, outputFile)
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
    "cpu_or_gpu": "GPU",
    "cpu_or_gpu_value": "QuadroP400",
    "extra_config": "RenderSKP",
    "model": "Golo",
    "os": "Ubuntu18"
  },
  "results": {
    "first_skp": {
      "webgl2": {
        "avg_render_frame_ms": 150.065,
        "avg_render_with_flush_ms": 133.91167,
        "avg_render_without_flush_ms": 45.398335,
        "median_render_frame_ms": 143.71,
        "median_render_with_flush_ms": 125.185,
        "median_render_without_flush_ms": 37.445,
        "skp_load_ms": 1.715,
        "stddev_render_frame_ms": 15.210527,
        "stddev_render_with_flush_ms": 15.47429,
        "stddev_render_without_flush_ms": 21.69691
      }
    },
    "second_skp": {
      "webgl2": {
        "avg_render_frame_ms": 316.7317,
        "avg_render_with_flush_ms": 233.91167,
        "avg_render_without_flush_ms": 85.39834,
        "median_render_frame_ms": 243.71,
        "median_render_with_flush_ms": 225.185,
        "median_render_without_flush_ms": 67.445,
        "skp_load_ms": 3.715,
        "stddev_render_frame_ms": 109.164635,
        "stddev_render_with_flush_ms": 15.474287,
        "stddev_render_without_flush_ms": 43.27188
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

const firstSKP = `{
  "without_flush_ms": [23.71, 37.445, 75.04],
  "with_flush_ms": [125.185, 120.895, 155.655],
  "total_frame_ms": [143.71, 135.445, 171.04],
  "skp_load_ms":1.715
}`

const secondSKP = `{
  "without_flush_ms": [43.71, 67.445, 145.04],
  "with_flush_ms": [225.185, 220.895, 255.655],
  "total_frame_ms": [243.71, 235.445, 471.04],
  "skp_load_ms":3.715
}`
