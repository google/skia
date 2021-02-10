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
	"strings"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/go/util"
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
		webGLVersion       = flag.String("webgl_version", "", "Major WebGL version to use when creating gl drawing context. 1 or 2")

		// Flags that may be required for certain configs
		canvaskitBinPath = flag.String("canvaskit_bin_path", "", "The location of a canvaskit.js and canvaskit.wasm")
		skpsPath         = flag.String("skps_path", "", "Path to location of skps.")

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
	skpsAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *skpsPath, "skps_path")
	outputAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *outputPath, "output_path")

	if err := setup(ctx, benchmarkAbsPath, nodeBinAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	if err := benchSKPs(ctx, outputWithoutResults, benchmarkAbsPath, canvaskitBinAbsPath, skpsAbsPath, nodeBinAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// outputFile name should be unique between tasks, so as to avoid having duplicate name files
	// uploaded to GCS.
	outputFile := filepath.Join(outputAbsPath, fmt.Sprintf("perf-%s.json", *taskID))
	if err := processSKPData(ctx, outputWithoutResults, benchmarkAbsPath, outputFile); err != nil {
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
	rv.Key["extra_config"] = "RenderSKP"
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

	// This is very important to make sure chrome is not running because we want to run chrome
	// with an unlocked framerate (see --disable-frame-rate-limit and --disable-gpu-vsync in
	// perf-canvaskit-with-puppeteer.js) and that won't happen if there is already an existing
	// chrome instance running when we try to run puppeteer. killall will return an error (e.g.
	// a non-zero error code) if there isn't already a chrome instance running. We can safely
	// ignore that error as we never expect there to be chrome running.
	_, _ = exec.RunSimple(ctx, "killall chrome")

	if err := os.MkdirAll(filepath.Join(benchmarkPath, "out"), 0777); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	return nil
}

var cpuSkiplist = []string{
	// When the SKPs were generated on Sept 27 2020, this started to timeout on CPU
	"desk_carsvg.skp",
	// This started timing out the week of Feb 10, 2021 on CPU.
	"desk_micrographygirlsvg.skp",
}
var gpuSkiplist = []string{}

// benchSKPs serves skps from a folder and runs the RenderSKPs benchmark on each of them
// individually. The benchmark is run N times to reduce the noise of the resulting data.
// The output for each will be a JSON file in $benchmarkPath/out/ corresponding to the skp name
// and the iteration it was.
func benchSKPs(ctx context.Context, perf perfJSONFormat, benchmarkPath, canvaskitBinPath, skpsPath, nodeBinPath string) error {
	ctx = td.StartStep(ctx, td.Props("perf skps in "+skpsPath))
	defer td.EndStep(ctx)

	// We expect the skpsPath to a directory with skp files in it.
	var skpFiles []string
	err := td.Do(ctx, td.Props("locate skpfiles"), func(ctx context.Context) error {
		return filepath.Walk(skpsPath, func(path string, info os.FileInfo, _ error) error {
			if path == skpsPath {
				return nil
			}
			if info.IsDir() {
				return filepath.SkipDir
			}
			skpFiles = append(skpFiles, path)
			return nil
		})
	})
	if err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	skiplist := cpuSkiplist
	if perf.Key[perfKeyCpuOrGPU] != "CPU" {
		skiplist = gpuSkiplist
	}
	sklog.Infof("Identified %d skp files to benchmark", len(skpFiles))

	for _, skp := range skpFiles {
		name := filepath.Base(skp)
		if util.In(name, skiplist) {
			sklog.Infof("Skipping skp %s", name)
			continue
		}
		err = td.Do(ctx, td.Props(fmt.Sprintf("Benchmark %s", name)), func(ctx context.Context) error {
			// See comment in setup about why we specify the absolute path for node.
			args := []string{filepath.Join(nodeBinPath, "node"),
				"perf-canvaskit-with-puppeteer",
				"--bench_html", "render-skp.html",
				"--canvaskit_js", filepath.Join(canvaskitBinPath, "canvaskit.js"),
				"--canvaskit_wasm", filepath.Join(canvaskitBinPath, "canvaskit.wasm"),
				"--timeout", "90", // 90 seconds before timeout
				"--input_skp", skp,
				"--output", filepath.Join(benchmarkPath, "out", name+".json"),
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
	}
	return nil
}

// TODO(kjlubick,jcgregorio) Could this code directly refer to the struct in Perf?
type perfJSONFormat struct {
	GitHash           string            `json:"gitHash"`
	SwarmingTaskID    string            `json:"swarming_task_id"`
	SwarmingMachineID string            `json:"swarming_machine_id"`
	Key               map[string]string `json:"key"`
	// Maps bench name -> "config" -> result key -> value
	Results map[string]map[string]perfResult `json:"results"`
}

type perfResult map[string]float32

// processSKPData looks at the result of benchSKPs, computes summary data on
// those files and adds them as Results into the provided perf object. The perf object is then
// written in JSON format to outputPath.
func processSKPData(ctx context.Context, perf perfJSONFormat, benchmarkPath, outputFilePath string) error {
	perfJSONPath := filepath.Join(benchmarkPath, "out")
	ctx = td.StartStep(ctx, td.Props("process perf output "+perfJSONPath))
	defer td.EndStep(ctx)

	var jsonInputs []string
	err := td.Do(ctx, td.Props("locate input JSON files"), func(ctx context.Context) error {
		return filepath.Walk(perfJSONPath, func(path string, info os.FileInfo, _ error) error {
			if strings.HasSuffix(path, ".json") {
				jsonInputs = append(jsonInputs, path)
				return nil
			}
			return nil
		})
	})
	if err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	sklog.Infof("Identified %d JSON inputs to process", len(jsonInputs))

	for _, skp := range jsonInputs {
		err = td.Do(ctx, td.Props("Process "+skp), func(ctx context.Context) error {
			name := strings.TrimSuffix(filepath.Base(skp), ".json")
			config := "software"
			if perf.Key[perfKeyCpuOrGPU] != "CPU" {
				config = "webgl2"
				if perf.Key[perfKeyWebGLVersion] == "1" {
					config = "webgl1"
				}
			}

			b, err := os_steps.ReadFile(ctx, skp)
			if err != nil {
				return skerr.Wrap(err)
			}
			metrics, err := parseSKPData(b)
			if err != nil {
				return skerr.Wrap(err)
			}

			perf.Results[name] = map[string]perfResult{
				config: metrics,
			}
			return nil
		})
		if err != nil {
			return td.FailStep(ctx, skerr.Wrap(err))
		}
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

type skpPerfData struct {
	WithoutFlushMS []float32 `json:"without_flush_ms"`
	WithFlushMS    []float32 `json:"with_flush_ms"`
	TotalFrameMS   []float32 `json:"total_frame_ms"`
	SKPLoadMS      float32   `json:"skp_load_ms"`
}

func parseSKPData(b []byte) (perfResult, error) {
	var data skpPerfData
	if err := json.Unmarshal(b, &data); err != nil {
		return nil, skerr.Wrap(err)
	}

	avgWithoutFlushMS, medianWithoutFlushMS, stddevWithoutFlushMS := summarize(data.WithoutFlushMS)
	avgWithFlushMS, medianWithFlushMS, stddevWithFlushMS := summarize(data.WithFlushMS)
	avgTotalFrameMS, medianTotalFrameMS, stddevTotalFrameMS := summarize(data.TotalFrameMS)

	return map[string]float32{
		"avg_render_without_flush_ms":    avgWithoutFlushMS,
		"median_render_without_flush_ms": medianWithoutFlushMS,
		"stddev_render_without_flush_ms": stddevWithoutFlushMS,

		"avg_render_with_flush_ms":    avgWithFlushMS,
		"median_render_with_flush_ms": medianWithFlushMS,
		"stddev_render_with_flush_ms": stddevWithFlushMS,

		"avg_render_frame_ms":    avgTotalFrameMS,
		"median_render_frame_ms": medianTotalFrameMS,
		"stddev_render_frame_ms": stddevTotalFrameMS,

		"skp_load_ms": data.SKPLoadMS,
	}, nil
}

func summarize(input []float32) (float32, float32, float32) {
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

	return avg, sorted[medIdx], stddev
}

func computeAverage(d []float32) float32 {
	avg := float32(0)
	for i := 0; i < len(d); i++ {
		avg += d[i]
	}
	avg /= float32(len(d))
	return avg
}
