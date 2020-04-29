// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
	"go.skia.org/infra/task_driver/go/td"
)

func main() {
	var (
		// Required properties for this task.
		projectID     = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskName      = flag.String("task_name", "", "Name of the task.")
		benchmarkPath = flag.String("benchmark_path", "", "Path to location of the benchmark files.")
		lottiesPath   = flag.String("lotties_path", "", "Path to location of lottie files.")
		workdir       = flag.String("workdir", ".", "Working directory")
		outputPath    = flag.String("output_path", "", "Perf Output will be produced here")
		gitHash       = flag.String("git_hash", "", "Git hash this data corresponds to")
		taskID        = flag.String("task_id", "", "task id this data was generated on")
		machineID     = flag.String("machine_id", "", "machine id this data was generated on")

		// Optional flags.
		local        = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		outputSteps  = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		canvaskitBin = flag.String("canvaskit_bin", "", "The location of a canvaskit.js and canvaskit.wasm")
	)

	// Setup.
	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	// FIXME(kjlubick) Remove this when done.
	if _, err := exec.RunCwd(ctx, *workdir, "ls", "-ahl"); err != nil {
		td.Fatal(ctx, err)
	}

	outputWithoutResults, err := makePerfObj(*taskName, *gitHash, *taskID, *machineID)
	if err != nil {
		td.Fatal(ctx, err)
	}

	if *benchmarkPath == "" {
		td.Fatalf(ctx, "--benchmark_path must be specified")
	}

	// Run the infra tests.
	if err := setup(ctx, *benchmarkPath); err != nil {
		td.Fatal(ctx, err)
	}

	parts := strings.Split(*taskName, "-")
	config := parts[len(parts)-1]
	switch config {
	case "Puppeteer_SkottieFrames_TaskDriver":
		if err := benchSkottieFrames(ctx, outputWithoutResults, *benchmarkPath, *canvaskitBin, *lottiesPath); err != nil {
			td.Fatal(ctx, err)
		}
		if err := processSkottieFramesData(ctx, outputWithoutResults, *benchmarkPath, *outputPath); err != nil {
			td.Fatal(ctx, err)
		}
	default:
		td.Fatalf(ctx, "Unknown task config %s [%s]", config, *taskName)
	}
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
	"", // filter
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
	if machineID == "" {
		return rv, skerr.Fmt("Must provide --machine_id")
	}
	rv.GitHash = gitHash
	rv.SwarmingTaskID = taskID
	rv.SwarmingMachineID = machineID

	rv.Key = map[string]string{
		"browser": "Chromium",
	}
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

func setup(ctx context.Context, benchmarkPath string) error {
	ctx = td.StartStep(ctx, td.Props("setup").Infra())
	defer td.EndStep(ctx)

	// FIXME(kjlubick)
	//if _, err := exec.RunCwd(ctx, benchmarkPath, "npm", "ci"); err != nil {
	//	return td.FailStep(ctx, err)
	//}

	if err := os.MkdirAll(filepath.Join(benchmarkPath, "out"), 0777); err != nil {
		return td.FailStep(ctx, err)
	}
	return nil
}

func benchSkottieFrames(ctx context.Context, perf perfJSONFormat, benchmarkPath, canvasKitBin, lottiesPath string) error {
	ctx = td.StartStep(ctx, td.Props("perf lotties in "+lottiesPath))
	if canvasKitBin == "" {
		return td.FailStep(ctx, skerr.Fmt("--canvaskit_bin must be specified"))
	}
	if lottiesPath == "" {
		return td.FailStep(ctx, skerr.Fmt("--lotties_path must be specified"))
	}
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
		return td.FailStep(ctx, err)
	}

	fmt.Printf("Identified %d lottie folders: %s\n", len(lottieFolders), lottieFolders)

	for _, lottie := range lottieFolders {
		name := filepath.Base(lottie)
		err = td.Do(ctx, td.Props("Benchmark "+name), func(ctx context.Context) error {
			args := []string{
				"node", "perf-with-puppeteer",
				"--bench_html", "canvaskit-skottie-frames-load.html",
				"--canvaskit_js", filepath.Join(canvasKitBin, "canvaskit.js"),
				"--canvaskit_wasm", filepath.Join(canvasKitBin, "canvaskit.wasm"),
				"--input", filepath.Join(lottie, "data.json"),
				"--assets", filepath.Join(lottie, "images"),
				"--output", filepath.Join(benchmarkPath, "out", name+".json"),
			}
			if perf.Key[cpuOrGPUKey] != "CPU" {
				args = append(args, "--use_gpu")
			}
			_, err := exec.RunCwd(ctx, benchmarkPath, args...)
			if err != nil {
				return td.FailStep(ctx, err)
			}
			return nil
		})
		if err != nil {
			return td.FailStep(ctx, err)
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
		return td.FailStep(ctx, err)
	}

	fmt.Printf("Identified %d JSON inputs to process\n", len(jsonInputs))

	for _, lottie := range jsonInputs {
		err = td.Do(ctx, td.Props("Process "+lottie), func(ctx context.Context) error {
			name := strings.TrimSuffix(filepath.Base(lottie), ".json")
			config := "software"
			if perf.Key[cpuOrGPUKey] != "CPU" {
				config = "webgl2"
			}
			b, err := ioutil.ReadFile(lottie)
			if err != nil {
				return td.FailStep(ctx, err)
			}
			metrics, err := parseSkottieFramesMetrics(b)
			if err != nil {
				return td.FailStep(ctx, err)
			}
			perf.Results[name] = map[string]perfResult{
				config: metrics,
			}
			return nil
		})
		if err != nil {
			return td.FailStep(ctx, err)
		}
	}

	err = td.Do(ctx, td.Props("Writing perf JSON file to "+outputPath), func(ctx context.Context) error {
		b, err := json.MarshalIndent(perf, "", "  ")
		if err != nil {
			return td.FailStep(ctx, err)
		}
		if err = ioutil.WriteFile(filepath.Join(outputPath, "perf.json"), b, 0666); err != nil {
			return td.FailStep(ctx, err)
		}
		return nil
	})
	if err != nil {
		return td.FailStep(ctx, err)
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
