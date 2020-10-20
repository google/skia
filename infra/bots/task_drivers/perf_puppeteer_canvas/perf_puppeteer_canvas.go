// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This executable is meant to be a general way to gather perf data using puppeteer. The logic
// (e.g. what bench to run, how to process that particular output) is selected using the ExtraConfig
// part of the task name.
package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"math"
	"os"
	"path/filepath"
	"sort"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

const perfKeyWebGLVersion = "webgl_version"

func main() {
	var (
		// Required properties for this task.
		projectID     = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskName      = flag.String("task_name", "", "Name of the task.")
		benchmarkPath = flag.String("benchmark_path", "", "Path to location of the benchmark files (e.g. //tools/perf-puppeteer).")
		outputPath    = flag.String("output_path", "", "Perf Output will be produced here")
		gitHash       = flag.String("git_hash", "", "Git hash this data corresponds to")
		taskID        = flag.String("task_id", "", "task id this data was generated on")
		nodeBinPath   = flag.String("node_bin_path", "", "Path to the node bin directory (should have npm also). This directory *must* be on the PATH when this executable is called, otherwise, the wrong node or npm version may be found (e.g. the one on the system), even if we are explicitly calling npm with the absolute path.")

		// These flags feed into the perf trace keys associated with the output data.
		osTrace            = flag.String("os_trace", "", "OS this is running on.")
		modelTrace         = flag.String("model_trace", "", "Description of host machine.")
		cpuOrGPUTrace      = flag.String("cpu_or_gpu_trace", "", "If this is a CPU or GPU configuration.")
		cpuOrGPUValueTrace = flag.String("cpu_or_gpu_value_trace", "", "The hardware of this CPU/GPU")
		webGLVersion       = flag.String("webgl_version", "", "Major WebGl version to use when creating gl drawing context. 1 or 2")

		// Flags that may be required for certain configs
		canvaskitBinPath = flag.String("canvaskit_bin_path", "", "The location of a canvaskit.js and canvaskit.wasm")

		// Debugging flags.
		local       = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		outputSteps = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
	)

	// Setup.
	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	keys := map[string]string{
		"os":                *osTrace,
		"model":             *modelTrace,
		perfKeyCpuOrGPU:     *cpuOrGPUTrace,
		"cpu_or_gpu_value":  *cpuOrGPUValueTrace,
		perfKeyWebGLVersion: *webGLVersion,
	}

	outputWithoutResults, err := makePerfObj(*gitHash, *taskID, os.Getenv("SWARMING_BOT_ID"), keys)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	// Absolute paths work more consistently than relative paths.
	nodeBinAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *nodeBinPath, "node_bin_path")
	benchmarkAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *benchmarkPath, "benchmark_path")
	canvaskitBinAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *canvaskitBinPath, "canvaskit_bin_path")
	outputAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *outputPath, "output_path")

	if err := setup(ctx, benchmarkAbsPath, nodeBinAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	if err := benchCanvas(ctx, outputWithoutResults, benchmarkAbsPath, canvaskitBinAbsPath, nodeBinAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// outputFile name should be unique between tasks, so as to avoid having duplicate name files
	// uploaded to GCS.
	outputFile := filepath.Join(outputAbsPath, fmt.Sprintf("perf-%s.json", *taskID))
	if err := processFramesData(ctx, outputWithoutResults, benchmarkAbsPath, outputFile); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
}

const perfKeyCpuOrGPU = "cpu_or_gpu"

func makePerfObj(gitHash, taskID, machineID string, keys map[string]string) (perfJSONFormat, error) {
	rv := perfJSONFormat{}
	if gitHash == "" {
		return rv, skerr.Fmt("Must provide --git_hash")
	}
	if taskID == "" {
		return rv, skerr.Fmt("Must provide --task_id")
	}
	rv.GitHash = gitHash
	rv.SwarmingTaskID = taskID
	rv.SwarmingMachineID = machineID
	rv.Key = keys
	rv.Key["arch"] = "wasm"
	rv.Key["browser"] = "Chromium"
	rv.Key["configuration"] = "Release"
	rv.Key["extra_config"] = "CanvasPerf"
	rv.Key["binary"] = "CanvasKit"
	rv.Results = map[string]map[string]perfResult{}
	return rv, nil
}

func setup(ctx context.Context, benchmarkPath, nodeBinPath string) error {
	ctx = td.StartStep(ctx, td.Props("setup").Infra())
	defer td.EndStep(ctx)

	if _, err := exec.RunCwd(ctx, benchmarkPath, filepath.Join(nodeBinPath, "npm"), "ci"); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	if err := os.MkdirAll(filepath.Join(benchmarkPath, "out"), 0777); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	return nil
}

// benchCanvas runs the puppeteer canvas_perf_driver.html test and parses the results.
func benchCanvas(ctx context.Context, perf perfJSONFormat, benchmarkPath, canvaskitBinPath, nodeBinPath string) error {
	ctx = td.StartStep(ctx, td.Props("perf canvas tests"))
	defer td.EndStep(ctx)

	err := td.Do(ctx, td.Props("Benchmark Canvas"), func(ctx context.Context) error {
		// See comment in setup about why we specify the absolute path for node.
		args := []string{filepath.Join(nodeBinPath, "node"),
			"perf-canvaskit-with-puppeteer",
			"--bench_html", "canvas_perf.html",
			"--canvaskit_js", filepath.Join(canvaskitBinPath, "canvaskit.js"),
			"--canvaskit_wasm", filepath.Join(canvaskitBinPath, "canvaskit.wasm"),
			"--assets", "canvas_perf_assets", // relative path
			"--output", filepath.Join(benchmarkPath, "out", "perf.json"),
			"--timeout", "240",
		}
		if perf.Key[perfKeyCpuOrGPU] != "CPU" {
			args = append(args, "--use_gpu")
			if perf.Key[perfKeyWebGLVersion] == "1" {
				args = append(args, "--query_params webgl1")
			}
		}

		_, err := exec.RunCwd(ctx, benchmarkPath, args...)
		if err != nil {
			return skerr.Wrap(err)
		}
		return nil
	})
	if err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	return nil
}

// description of the output file format
type perfJSONFormat struct {
	GitHash           string            `json:"gitHash"`
	SwarmingTaskID    string            `json:"swarming_task_id"`
	SwarmingMachineID string            `json:"swarming_machine_id"`
	Key               map[string]string `json:"key"`
	// Maps bench name -> "config" -> result key -> value
	Results map[string]map[string]perfResult `json:"results"`
}

type perfResult map[string]float32

// description of the input file format.
type oneTestResult struct {
	WithoutFlushMS []float32 `json:"without_flush_ms"`
	WithFlushMS    []float32 `json:"with_flush_ms"`
	TotalFrameMS   []float32 `json:"total_frame_ms"`
}

// processFramesData looks at the result of benchCanvas, computes summary data on
// those files and adds them as Results into the provided perf object. The perf object is then
// written in JSON format to outputPath.
func processFramesData(ctx context.Context, perf perfJSONFormat, benchmarkPath, outputFilePath string) error {
	perfJSONPath := filepath.Join(benchmarkPath, "out", "perf.json")
	ctx = td.StartStep(ctx, td.Props("process perf output "+perfJSONPath))
	defer td.EndStep(ctx)

	err := td.Do(ctx, td.Props("Process "+perfJSONPath), func(ctx context.Context) error {
		config := "software"
		if perf.Key[perfKeyCpuOrGPU] != "CPU" {
			config = "webgl2"
			if perf.Key[perfKeyWebGLVersion] == "1" {
				config = "webgl1"
			}
		}
		b, err := os_steps.ReadFile(ctx, perfJSONPath)
		if err != nil {
			return skerr.Wrap(err)
		}
		var fileData map[string]oneTestResult
		if err := json.Unmarshal(b, &fileData); err != nil {
			return skerr.Wrap(err)
		}

		for name, item := range fileData {
			metrics, err := calculatePerfFromTest(item) // item is a oneTestResult
			if err != nil {
				return skerr.Wrap(err)
			}
			perf.Results[name] = map[string]perfResult{
				config: metrics,
			}
		}
		return nil
	})
	if err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	err = td.Do(ctx, td.Props("Writing perf JSON file to "+outputFilePath), func(ctx context.Context) error {
		if err := os.MkdirAll(filepath.Dir(outputFilePath), 0777); err != nil {
			return skerr.Wrap(err)
		}
		b, err := json.MarshalIndent(perf, "", "  ")
		if err != nil {
			return skerr.Wrap(err)
		}
		if err = ioutil.WriteFile(outputFilePath, b, 0666); err != nil {
			return skerr.Wrap(err)
		}
		return nil
	})
	if err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	return nil
}

// Computer averages and quantiles of the frame time results from one test.
func calculatePerfFromTest(metrics oneTestResult) (map[string]float32, error) {
	avgWithoutFlushMS, medianWithoutFlushMS, stddevWithoutFlushMS, _, _, _ := summarize(metrics.WithoutFlushMS)
	avgWithFlushMS, medianWithFlushMS, stddevWithFlushMS, _, _, _ := summarize(metrics.WithFlushMS)
	avgFrame, medFrame, stdFrame, percentile90Frame, percentile95Frame, percentile99Frame := summarize(metrics.TotalFrameMS)

	rv := map[string]float32{
		"avg_render_without_flush_ms":    avgWithoutFlushMS,
		"median_render_without_flush_ms": medianWithoutFlushMS,
		"stddev_render_without_flush_ms": stddevWithoutFlushMS,

		"avg_render_with_flush_ms":    avgWithFlushMS,
		"median_render_with_flush_ms": medianWithFlushMS,
		"stddev_render_with_flush_ms": stddevWithFlushMS,

		"avg_render_frame_ms":    avgFrame,
		"median_render_frame_ms": medFrame,
		"stddev_render_frame_ms": stdFrame,

		// more detailed statistics on total frame times
		"90th_percentile_frame_ms": percentile90Frame,
		"95th_percentile_frame_ms": percentile95Frame,
		"99th_percentile_frame_ms": percentile99Frame,
	}
	return rv, nil
}

func summarize(input []float32) (float32, float32, float32, float32, float32, float32) {
	// Make a copy of the data so we don't mutate the order of the original
	sorted := make([]float32, len(input))
	copy(sorted, input)
	sort.Slice(sorted, func(i, j int) bool {
		return sorted[i] < sorted[j]
	})

	avg := computeAverage(sorted)
	variance := float32(0)
	for i := 0; i < len(sorted); i++ {
		variance += (sorted[i] - avg) * (sorted[i] - avg)
	}
	stddev := float32(math.Sqrt(float64(variance / float32(len(sorted)))))

	medIdx := (len(sorted) * 50) / 100
	percentile90Idx := (len(sorted) * 90) / 100
	percentile95Idx := (len(sorted) * 95) / 100
	percentile99Idx := (len(sorted) * 99) / 100

	return avg, sorted[medIdx], stddev, sorted[percentile90Idx], sorted[percentile95Idx], sorted[percentile99Idx]
}

func computeAverage(d []float32) float32 {
	avg := float32(0)
	for i := 0; i < len(d); i++ {
		avg += d[i]
	}
	avg /= float32(len(d))
	return avg
}
