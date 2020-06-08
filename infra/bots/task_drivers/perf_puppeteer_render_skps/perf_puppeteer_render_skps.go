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
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

const (
	// numTimes is how many times we benchmark a single SKP. We can't do these benchmarks all in the
	// same Chrome window because stuff is cached and we don't get very reliable data. For example,
	// the first time to render a given skp might be 100ms and then the second time and beyond are few
	// millisconds. This number was chosen arbitrarily to make sure we have sufficient samples, such
	// that we don't take too long, and is odd so our naive median calculation works without needing
	// to average the middle two points.
	numTimes = 11
)

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
		"os":               *osTrace,
		"model":            *modelTrace,
		perfKeyCpuOrGPU:    *cpuOrGPUTrace,
		"cpu_or_gpu_value": *cpuOrGPUValueTrace,
	}

	outputWithoutResults, err := makePerfObj(*gitHash, *taskID, os.Getenv("SWARMING_BOT_ID"), keys)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	// Absolute paths work more consistently than relative paths.
	nodeBinAbsPath := getAbsoluteOfRequiredFlag(ctx, *nodeBinPath, "node_bin_path")
	benchmarkAbsPath := getAbsoluteOfRequiredFlag(ctx, *benchmarkPath, "benchmark_path")
	canvaskitBinAbsPath := getAbsoluteOfRequiredFlag(ctx, *canvaskitBinPath, "canvaskit_bin_path")
	skpsAbsPath := getAbsoluteOfRequiredFlag(ctx, *skpsPath, "skps_path")
	outputAbsPath := getAbsoluteOfRequiredFlag(ctx, *outputPath, "output_path")

	// Run the infra tests.
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

func getAbsoluteOfRequiredFlag(ctx context.Context, nonEmptyPath, flag string) string {
	if nonEmptyPath == "" {
		td.Fatalf(ctx, "--%s must be specified", flag)
	}
	absPath, err := filepath.Abs(nonEmptyPath)
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	return absPath
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

	if err := os.MkdirAll(filepath.Join(benchmarkPath, "out"), 0777); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	return nil
}

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

	sklog.Infof("Identified %d skp files to benchmark", len(skpFiles))

	for _, skp := range skpFiles {
		name := filepath.Base(skp)
		for i := 1; i <= numTimes; i++ {
			output := fmt.Sprintf("%s_%03d.json", name, i)
			err = td.Do(ctx, td.Props(fmt.Sprintf("Benchmark %d %s", i, name)), func(ctx context.Context) error {
				// See comment in setup about why we specify the absolute path for node.
				args := []string{filepath.Join(nodeBinPath, "node"),
					"perf-canvaskit-with-puppeteer",
					"--bench_html", "render-skp.html",
					"--canvaskit_js", filepath.Join(canvaskitBinPath, "canvaskit.js"),
					"--canvaskit_wasm", filepath.Join(canvaskitBinPath, "canvaskit.wasm"),
					"--input_skp", skp,
					"--output", filepath.Join(benchmarkPath, "out", output),
				}
				if perf.Key[perfKeyCpuOrGPU] != "CPU" {
					args = append(args, "--use_gpu")
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
	}
	return nil
}

type perfJSONFormat struct {
	GitHash           string            `json:"gitHash"`
	SwarmingTaskID    string            `json:"swarming_task_id"`
	SwarmingMachineID string            `json:"swarming_machine_id"`
	Key               map[string]string `json:"key"`
	// Maps bench name -> "config" -> result key -> value
	Results map[string]map[string]perfResult `json:"results"`
}

type perfResult map[string]interface{}

// processSKPData looks at the result of benchSKPs, computes summary data on
// those files and adds them as Results into the provided perf object. The perf object is then
// written in JSON format to outputPath.
func processSKPData(ctx context.Context, perf perfJSONFormat, benchmarkPath, outputFilePath string) error {
	perfJSONPath := filepath.Join(benchmarkPath, "out")
	ctx = td.StartStep(ctx, td.Props("process perf output "+perfJSONPath))
	defer td.EndStep(ctx)

	jsonGroupedBySKP := map[string][]string{}
	err := td.Do(ctx, td.Props("locate input JSON files"), func(ctx context.Context) error {
		const suffixLength = len("_000.json")
		return filepath.Walk(perfJSONPath, func(path string, info os.FileInfo, _ error) error {
			if strings.HasSuffix(path, ".json") {
				skpName := filepath.Base(path)
				skpName = skpName[:len(skpName)-suffixLength]
				jsonGroupedBySKP[skpName] = append(jsonGroupedBySKP[skpName], path)
				return nil
			}
			return nil
		})
	})
	if err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	sklog.Infof("Identified %d JSON inputs to process", len(jsonGroupedBySKP))

	for skp, jsonFiles := range jsonGroupedBySKP {
		err = td.Do(ctx, td.Props("Process "+skp), func(ctx context.Context) error {
			name := skp
			config := "software"
			if perf.Key[perfKeyCpuOrGPU] != "CPU" {
				config = "webgl2"
			}

			var rawMetrics []skpPerfData
			for _, jsonFile := range jsonFiles {
				b, err := os_steps.ReadFile(ctx, jsonFile)
				if err != nil {
					return skerr.Wrap(err)
				}
				metrics, err := parseSKPData(b)
				if err != nil {
					return skerr.Wrap(err)
				}
				rawMetrics = append(rawMetrics, metrics)
			}

			perf.Results[name] = map[string]perfResult{
				config: makeResults(rawMetrics),
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
	WithoutFlushMS float32 `json:"without_flush_ms"`
	WithFlushMS    float32 `json:"with_flush_ms"`
	JSONLoadMS     float32 `json:"json_load_ms"`
}

func parseSKPData(b []byte) (skpPerfData, error) {
	var metrics skpPerfData
	if err := json.Unmarshal(b, &metrics); err != nil {
		return skpPerfData{}, skerr.Wrap(err)
	}
	return metrics, nil
}

func makeResults(data []skpPerfData) perfResult {
	var withoutFlushes []float32
	var withFlushes []float32
	var loads []float32
	for _, d := range data {
		withoutFlushes = append(withoutFlushes, d.WithoutFlushMS)
		withFlushes = append(withFlushes, d.WithFlushMS)
		loads = append(loads, d.JSONLoadMS)
	}

	avgWithoutFlushMS, medianWithoutFlushMS, stddevWithoutFlushMS := summarize(withoutFlushes)
	avgWithFlushMS, medianWithFlushMS, stddevWithFlushMS := summarize(withFlushes)
	avgLoadMS, medianLoadMS, stddevLoadMS := summarize(loads)
	return map[string]interface{}{
		"avg_render_without_flush_ms":    avgWithoutFlushMS,
		"median_render_without_flush_ms": medianWithoutFlushMS,
		"stddev_render_without_flush_ms": stddevWithoutFlushMS,

		"avg_render_with_flush_ms":     avgWithFlushMS,
		"median_render_with_flush_mss": medianWithFlushMS,
		"stddev_render_with_flush_ms":  stddevWithFlushMS,

		"avg_json_load_ms":    avgLoadMS,
		"median_json_load_ms": medianLoadMS,
		"stddev_json_load_ms": stddevLoadMS,

		"render_without_flush_ms_samples": withoutFlushes,
		"render_with_flush_ms_samples":    withFlushes,
		"render_json_load_ms_samples":     loads,
	}
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
