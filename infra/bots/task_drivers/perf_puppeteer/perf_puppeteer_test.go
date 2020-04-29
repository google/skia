package main

import (
	"context"
	"io/ioutil"
	"os"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"go.skia.org/infra/task_driver/go/td"
)

func testCtx() context.Context {
	whatever := "whatever"
	blank := ""
	truePtr := true
	// TODO(borenet): Add something to easily make a task_driver context for tests.
	return td.StartRun(&whatever, &whatever, &whatever, &blank, &truePtr)
}

func TestProcessSkottieFramesData_CPUTwoInputsGetJoinedTogether(t *testing.T) {
	const taskName = "Perf-Debian10-EMCC-GCE-CPU-AVX2-wasm-Release-All-Puppeteer_SkottieFrames_TaskDriver"

	input, err := ioutil.TempDir("", "inputs")
	require.NoError(t, err)
	err = writeFilesToDisk(filepath.Join(input, "out"), map[string]string{
		"first_animation.json":  skottieFramesSampleOne,
		"second_animation.json": skottieFramesSampleTwo,
	})
	require.NoError(t, err)
	output, err := ioutil.TempDir("", "perf")
	require.NoError(t, err)

	perfObj, err := makePerfObj(taskName, someGitHash, someTaskID, someMachineID)
	require.NoError(t, err)

	err = processSkottieFramesData(testCtx(), perfObj, input, output)
	require.NoError(t, err)

	b, err := ioutil.ReadFile(filepath.Join(output, "perf.json"))
	require.NoError(t, err)

	assert.Equal(t, `{
    "gitHash": "032631e490db494128e0610a19adce4cab9706d1", 
    "swarming_task_id": "4bdd43ed7c906c11",
    "swarming_machine_id": "skia-e-gce-203"
    "key": {
        "browser": "Chromium",
        "compiler": "EMCC",
        "configuration": "Release", 
        "cpu_or_gpu_value": "AVX2", 
        "os": "Debian10", 
        "model": "GCE", 
        "cpu_or_gpu": "CPU", 
        "arch": "wasm", 
        "extra_config": "SkottieFrames"
    }, 
    "results": {
        "first_animation": {
            "software": {
                "json_load_ms": 16.05,
                "avg_frame_to_flush_ms": 47992.0, 
                "median_frame_to_flush_ms": 37782.0,
                "stddev_frame_to_flush_ms": 37782.0,
                "90th_percentile_frame_to_flush_ms": 37782.0,
                "95th_percentile_frame_to_flush_ms": 37782.0,
                "99th_percentile_frame_to_flush_ms": 37782.0,
                "1st_frame_to_flush_ms" : 31.555,
                "2nd_frame_to_flush_ms" : 87.795
                "3rd_frame_to_flush_ms" : 31.555,
                "4th_frame_to_flush_ms" : 31.555,
                "5th_frame_to_flush_ms" : 31.555,
                "avg_first_five_frames_ms": 3,
                "avg_seek_ms": 47992.0, 
                "median_seek_ms": 37782.0,
                "stddev_seek_ms": 37782.0
            }
        }, 
        "second_animation": {
            "software": {
               "json_load_ms": 16.05,
                "avg_frame_to_flush_ms": 47992.0, 
                "median_frame_to_flush_ms": 37782.0,
                "stddev_frame_to_flush_ms": 37782.0,
                "90th_percentile_frame_to_flush_ms": 37782.0,
                "95th_percentile_frame_to_flush_ms": 37782.0,
                "99th_percentile_frame_to_flush_ms": 37782.0,
                "1st_frame_to_flush_ms" : 31.555,
                "2nd_frame_to_flush_ms" : 87.795
                "3rd_frame_to_flush_ms" : 31.555,
                "4th_frame_to_flush_ms" : 31.555,
                "5th_frame_to_flush_ms" : 31.555,
                "avg_first_five_frames_ms": 3,
                "avg_seek_ms": 47992.0, 
                "median_seek_ms": 37782.0,
                "stddev_seek_ms": 37782.0
            }
        },
    }
}
`, string(b))
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
  "jsonLoad_ms": 16.05
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
  "jsonLoad_ms": 28.15
}`
