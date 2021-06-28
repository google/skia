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
		webGLVersion       = flag.String("webgl_version", "", "Major WebGl version to use when creating gl drawing context. 1 or 2")

		// Flags that may be required for certain configs
		canvaskitBinPath = flag.String("canvaskit_bin_path", "", "The location of a canvaskit.js and canvaskit.wasm")
		lottiesPath      = flag.String("lotties_path", "", "Path to location of lottie files.")

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
	lottiesAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *lottiesPath, "lotties_path")
	outputAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *outputPath, "output_path")

	if err := setup(ctx, benchmarkAbsPath, nodeBinAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	if err := benchSkottieFrames(ctx, outputWithoutResults, benchmarkAbsPath, canvaskitBinAbsPath, lottiesAbsPath, nodeBinAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// outputFile name should be unique between tasks, so as to avoid having duplicate name files
	// uploaded to GCS.
	outputFile := filepath.Join(outputAbsPath, fmt.Sprintf("perf-%s.json", *taskID))
	if err := processSkottieFramesData(ctx, outputWithoutResults, benchmarkAbsPath, outputFile); err != nil {
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
	rv.Key["extra_config"] = "SkottieFrames"
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

var cpuSkiplist = []string{
	"Curly_Hair",                       // Times out after drawing ~200 frames.
	"Day_Night",                        // Times out after drawing ~400 frames.
	"an_endless_hike_on_a_tiny_world_", // Times out after drawing ~200 frames.
	"beetle",                           // Times out after drawing ~500 frames.
	"day_night_cycle",                  // Times out after drawing ~400 frames.
	"fidget_spinner",                   // Times out after drawing ~400 frames.
	"intelia_logo_animation",           // Times out after drawing ~300 frames.
	"siren",                            // Times out after drawing ~500 frames.
	"truecosmos",                       // Times out after drawing ~200 frames.
}
var gpuSkiplist = []string{}

// benchSkottieFrames serves lotties and assets from a folder and runs the skottie-frames-load
// benchmark on each of them individually. The output for each will be a JSON file in
// $benchmarkPath/out/ corresponding to the animation name.
func benchSkottieFrames(ctx context.Context, perf perfJSONFormat, benchmarkPath, canvaskitBinPath, lottiesPath, nodeBinPath string) error {
	ctx = td.StartStep(ctx, td.Props("perf lotties in "+lottiesPath))
	defer td.EndStep(ctx)

	// We expect the lottiesPath to be a series of folders, each with a data.json and a subfolder of
	// images. For example:
	// lottiesPath
	//    /first-animation/
	//       data.json
	//       /images/
	//          img001.png
	//          img002.png
	//          my-font.ttf
	var lottieFolders []string
	err := td.Do(ctx, td.Props("locate lottie folders"), func(ctx context.Context) error {
		return filepath.Walk(lottiesPath, func(path string, info os.FileInfo, _ error) error {
			if path == lottiesPath {
				return nil
			}
			if info.IsDir() {
				lottieFolders = append(lottieFolders, path)
				return filepath.SkipDir
			}
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

	sklog.Infof("Identified %d lottie folders to benchmark", len(lottieFolders))

	var lastErr error
	for _, lottie := range lottieFolders {
		name := filepath.Base(lottie)
		if util.In(name, skiplist) {
			sklog.Infof("Skipping lottie %s", name)
			continue
		}
		err = td.Do(ctx, td.Props("Benchmark "+name), func(ctx context.Context) error {
			// See comment in setup about why we specify the absolute path for node.
			args := []string{filepath.Join(nodeBinPath, "node"),
				"perf-canvaskit-with-puppeteer",
				"--bench_html", "skottie-frames.html",
				"--canvaskit_js", filepath.Join(canvaskitBinPath, "canvaskit.js"),
				"--canvaskit_wasm", filepath.Join(canvaskitBinPath, "canvaskit.wasm"),
				"--input_lottie", filepath.Join(lottie, "data.json"),
				"--assets", filepath.Join(lottie, "images"),
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
			lastErr = td.FailStep(ctx, skerr.Wrap(err))
			// Don't return - we want to try to test all the inputs.
		}
	}
	return lastErr // will be nil if no lottie failed.
}

type perfJSONFormat struct {
	GitHash           string            `json:"gitHash"`
	SwarmingTaskID    string            `json:"swarming_task_id"`
	SwarmingMachineID string            `json:"swarming_machine_id"`
	Key               map[string]string `json:"key"`
	// Maps bench name -> "config" -> result key -> value
	Results map[string]map[string]perfResult `json:"results"`
}

type perfResult map[string]float32

// processSkottieFramesData looks at the result of benchSkottieFrames, computes summary data on
// those files and adds them as Results into the provided perf object. The perf object is then
// written in JSON format to outputPath.
func processSkottieFramesData(ctx context.Context, perf perfJSONFormat, benchmarkPath, outputFilePath string) error {
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

	for _, lottie := range jsonInputs {
		err = td.Do(ctx, td.Props("Process "+lottie), func(ctx context.Context) error {
			name := strings.TrimSuffix(filepath.Base(lottie), ".json")
			config := "software"
			if perf.Key[perfKeyCpuOrGPU] != "CPU" {
				config = "webgl2"
				if perf.Key[perfKeyWebGLVersion] == "1" {
					config = "webgl1"
				}
			}
			b, err := os_steps.ReadFile(ctx, lottie)
			if err != nil {
				return skerr.Wrap(err)
			}
			metrics, err := parseSkottieFramesMetrics(b)
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

type skottieFramesJSONFormat struct {
	WithoutFlushMS []float32 `json:"without_flush_ms"`
	WithFlushMS    []float32 `json:"with_flush_ms"`
	TotalFrameMS   []float32 `json:"total_frame_ms"`
	JSONLoadMS     float32   `json:"json_load_ms"`
}

func parseSkottieFramesMetrics(b []byte) (map[string]float32, error) {
	var metrics skottieFramesJSONFormat
	if err := json.Unmarshal(b, &metrics); err != nil {
		return nil, skerr.Wrap(err)
	}

	getNthFrame := func(n int) float32 {
		if n >= len(metrics.TotalFrameMS) {
			return 0
		}
		return metrics.TotalFrameMS[n]
	}

	avgFirstFive := float32(0)
	if len(metrics.TotalFrameMS) >= 5 {
		avgFirstFive = computeAverage(metrics.TotalFrameMS[:5])
	}

	avgWithoutFlushMS, medianWithoutFlushMS, stddevWithoutFlushMS, _, _, _ := summarize(metrics.WithoutFlushMS)
	avgWithFlushMS, medianWithFlushMS, stddevWithFlushMS, _, _, _ := summarize(metrics.WithFlushMS)
	avgFrame, medFrame, stdFrame, p90Frame, p95Frame, p99Frame := summarize(metrics.TotalFrameMS)

	rv := map[string]float32{
		"json_load_ms": metrics.JSONLoadMS,

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
		"1st_frame_ms":             getNthFrame(0),
		"2nd_frame_ms":             getNthFrame(1),
		"3rd_frame_ms":             getNthFrame(2),
		"4th_frame_ms":             getNthFrame(3),
		"5th_frame_ms":             getNthFrame(4),
		"avg_first_five_frames_ms": avgFirstFive,
		"90th_percentile_frame_ms": p90Frame,
		"95th_percentile_frame_ms": p95Frame,
		"99th_percentile_frame_ms": p99Frame,
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
	p90Idx := (len(sorted) * 90) / 100
	p95Idx := (len(sorted) * 95) / 100
	p99Idx := (len(sorted) * 99) / 100

	return avg, sorted[medIdx], stddev, sorted[p90Idx], sorted[p95Idx], sorted[p99Idx]
}

func computeAverage(d []float32) float32 {
	avg := float32(0)
	for i := 0; i < len(d); i++ {
		avg += d[i]
	}
	avg /= float32(len(d))
	return avg
}
