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
	"math/rand"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"time"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/task_driver/go/td"
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

		// Flags that may be required for certain configs
		canvaskitBinPath = flag.String("canvaskit_bin_path", "", "The location of a canvaskit.js and canvaskit.wasm")
		lottiesPath      = flag.String("lotties_path", "", "Path to location of lottie files.")

		// Debugging flags.
		local       = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		outputSteps = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
	)

	rand.Seed(time.Now().UnixNano())

	// Setup.
	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	outputWithoutResults, err := makePerfObj(*taskName, *gitHash, *taskID, os.Getenv("SWARMING_BOT_ID"))
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}
	// Absolute paths work more consistently than relative paths.
	nodeBinAbsPath := getAbsoluteOfRequiredFlag(ctx, *nodeBinPath, "node_bin_path")
	benchmarkAbsPath := getAbsoluteOfRequiredFlag(ctx, *benchmarkPath, "benchmark_path")

	// Run the infra tests.
	if err := setup(ctx, benchmarkAbsPath, nodeBinAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	parts := strings.Split(*taskName, "-")
	config := parts[len(parts)-1]
	switch config {
	case "Puppeteer_SkottieFrames_TaskDriver":
		canvaskitBinAbsPath := getAbsoluteOfRequiredFlag(ctx, *canvaskitBinPath, "canvaskit_bin_path")
		lottiesAbsPath := getAbsoluteOfRequiredFlag(ctx, *lottiesPath, "lotties_path")
		outputAbsPath := getAbsoluteOfRequiredFlag(ctx, *outputPath, "output_path")
		if err := benchSkottieFrames(ctx, outputWithoutResults, benchmarkAbsPath, canvaskitBinAbsPath, lottiesAbsPath, nodeBinAbsPath); err != nil {
			td.Fatal(ctx, skerr.Wrap(err))
		}
		if err := processSkottieFramesData(ctx, outputWithoutResults, benchmarkAbsPath, outputAbsPath); err != nil {
			td.Fatal(ctx, skerr.Wrap(err))
		}
	default:
		td.Fatalf(ctx, "Unknown task config %s [%s]", config, *taskName)
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

var perfKeys = []string{
	"", // Perf
	"os",
	"compiler",
	"model",
	"cpu_or_gpu",
	"cpu_or_gpu_value",
	"arch",
	"configuration",
	"", // filter, which we ignore
}

const cpuOrGPUKey = "cpu_or_gpu"

func makePerfObj(taskName, gitHash, taskID, machineID string) (perfJSONFormat, error) {
	rv := perfJSONFormat{}
	if taskName == "" {
		return rv, skerr.Fmt("Must provide --task_name")
	}
	if gitHash == "" {
		return rv, skerr.Fmt("Must provide --git_hash")
	}
	if taskID == "" {
		return rv, skerr.Fmt("Must provide --task_id")
	}
	rv.GitHash = gitHash
	rv.SwarmingTaskID = taskID
	rv.SwarmingMachineID = machineID

	rv.Key = map[string]string{
		"browser": "Chromium",
	}
	// This is a quick and dirty clone of the more general schema code from the gen_tasks_logic.
	// I didn't use that because I didn't want to have to deal with reading in a JSON file that
	// contained the schema.
	sections := strings.Split(taskName, "-")
	for i, key := range perfKeys {
		if key == "" {
			continue
		}
		rv.Key[key] = sections[i]
	}

	// Special case for extra_config, which looks like
	// "Puppeteer_SkottieFrames_TaskDriver" and we want to trim it down.
	extraConfig := sections[len(sections)-1]
	extraConfig = strings.TrimPrefix(extraConfig, "Puppeteer_")
	extraConfig = strings.TrimSuffix(extraConfig, "_TaskDriver")
	rv.Key["extra_config"] = extraConfig

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

	sklog.Infof("Identified %d lottie folders to benchmark", len(lottieFolders))

	for _, lottie := range lottieFolders {
		name := filepath.Base(lottie)
		err = td.Do(ctx, td.Props("Benchmark "+name), func(ctx context.Context) error {
			// See comment in setup about why we specify the absolute path for node.
			args := []string{filepath.Join(nodeBinPath, "node"),
				"perf-with-puppeteer",
				"--bench_html", "canvaskit-skottie-frames-load.html",
				"--canvaskit_js", filepath.Join(canvaskitBinPath, "canvaskit.js"),
				"--canvaskit_wasm", filepath.Join(canvaskitBinPath, "canvaskit.wasm"),
				"--input", filepath.Join(lottie, "data.json"),
				"--assets", filepath.Join(lottie, "images"),
				"--output", filepath.Join(benchmarkPath, "out", name+".json"),
			}
			if perf.Key[cpuOrGPUKey] != "CPU" {
				args = append(args, "--use_gpu")
			}
			_, err := exec.RunCwd(ctx, benchmarkPath, args...)
			if err != nil {
				return td.FailStep(ctx, skerr.Wrap(err))
			}
			return nil
		})
		if err != nil {
			return td.FailStep(ctx, skerr.Wrap(err))
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

type perfResult map[string]float32

// processSkottieFramesData looks at the result of benchSkottieFrames, computes summary data on
// those files and adds them as Results into the provided perf object. The perf object is then
// written in JSON format to outputPath.
func processSkottieFramesData(ctx context.Context, perf perfJSONFormat, benchmarkPath, outputPath string) error {
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
			if perf.Key[cpuOrGPUKey] != "CPU" {
				config = "webgl2"
			}
			b, err := ioutil.ReadFile(lottie)
			if err != nil {
				return td.FailStep(ctx, skerr.Wrap(err))
			}
			metrics, err := parseSkottieFramesMetrics(b)
			if err != nil {
				return td.FailStep(ctx, skerr.Wrap(err))
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

	err = td.Do(ctx, td.Props("Writing perf JSON file to "+outputPath), func(ctx context.Context) error {
		if err := os.MkdirAll(outputPath, 0777); err != nil {
			return td.FailStep(ctx, skerr.Wrap(err))
		}
		b, err := json.MarshalIndent(perf, "", "  ")
		if err != nil {
			return td.FailStep(ctx, skerr.Wrap(err))
		}
		if err = ioutil.WriteFile(filepath.Join(outputPath, getRandomOutputName()), b, 0666); err != nil {
			return td.FailStep(ctx, skerr.Wrap(err))
		}
		return nil
	})
	if err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	return nil
}

type skottieFramesJSONFormat struct {
	FramesMS   []float32 `json:"frames_ms"`
	SeeksMS    []float32 `json:"seeks_ms"`
	JSONLoadMS float32   `json:"json_load_ms"`
}

func parseSkottieFramesMetrics(b []byte) (map[string]float32, error) {
	var metrics skottieFramesJSONFormat
	if err := json.Unmarshal(b, &metrics); err != nil {
		return nil, skerr.Wrap(err)
	}

	getNthFrame := func(n int) float32 {
		if n >= len(metrics.FramesMS) {
			return 0
		}
		return metrics.FramesMS[n]
	}

	avgFirstFive := float32(0)
	if len(metrics.FramesMS) >= 5 {
		avgFirstFive = computeAverage(metrics.FramesMS[:5])
	}

	avgFrame, medFrame, stdFrame, p90Frame, p95Frame, p99Frame := summarize(metrics.FramesMS)
	avgSeek, medSeek, stdSeek, _, _, _ := summarize(metrics.SeeksMS)

	rv := map[string]float32{
		"json_load_ms":             metrics.JSONLoadMS,
		"1st_frame_to_flush_ms":    getNthFrame(0),
		"2nd_frame_to_flush_ms":    getNthFrame(1),
		"3rd_frame_to_flush_ms":    getNthFrame(2),
		"4th_frame_to_flush_ms":    getNthFrame(3),
		"5th_frame_to_flush_ms":    getNthFrame(4),
		"avg_first_five_frames_ms": avgFirstFive,

		"avg_frame_to_flush_ms":             avgFrame,
		"median_frame_to_flush_ms":          medFrame,
		"stddev_frame_to_flush_ms":          stdFrame,
		"90th_percentile_frame_to_flush_ms": p90Frame,
		"95th_percentile_frame_to_flush_ms": p95Frame,
		"99th_percentile_frame_to_flush_ms": p99Frame,

		"avg_seek_ms":    avgSeek,
		"median_seek_ms": medSeek,
		"stddev_seek_ms": stdSeek,
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

// getRandomOutputName returns a randomized json name to minimize the risk that we have two
// different perf jobs running at the same commit or patchset and overwrite each others data.
// By default the file name we choose here is unchanged as it is uploaded into the given perf
// directory, so we inject some randomness into the name.
func getRandomOutputName() string {
	return fmt.Sprintf("perf-%d.json", rand.Int63())
}
