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
	"os"
	"path/filepath"
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

	// Run the infra tests.
	if err := setup(ctx, *benchmarkPath); err != nil {
		td.Fatal(ctx, err)
	}

	parts := strings.Split(*taskName, "-")
	config := parts[len(parts)-1]
	switch config {
	case "Puppeteer_SkottieFrames_TaskDriver":
		if err := benchSkottieFrames(ctx, *benchmarkPath, *canvaskitBin, *lottiesPath); err != nil {
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

func benchSkottieFrames(ctx context.Context, benchmarkPath, canvasKitBin, lottiesPath string) error {
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
		return td.FailStep(ctx, err)
	}

	fmt.Printf("Identified %d lottie folders: %s\n", len(lottieFolders), lottieFolders)

	for _, lottie := range lottieFolders {
		name := filepath.Base(lottie)
		err = td.Do(ctx, td.Props("Benchmark "+name), func(ctx context.Context) error {
			_, err := exec.RunCwd(ctx, benchmarkPath, "node", "perf-with-puppeteer",
				"--bench_html", "canvaskit-skottie-frames-load.html",
				"--canvaskit_js", filepath.Join(canvasKitBin, "canvaskit.js"),
				"--canvaskit_wasm", filepath.Join(canvasKitBin, "canvaskit.wasm"),
				"--input", filepath.Join(lottie, "data.json"),
				"--assets", filepath.Join(lottie, "images"),
				"--output", filepath.Join(benchmarkPath, "out", name+".json"),
				"--use_gpu", // TODO(kjlubick)
			)
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
		name := filepath.Base(lottie)
		config := "software"
		if perf.Key["cpu_or_gpu"] != "CPU" {
			config = "webgl2"
		}
		perf.Results[name] = map[string]perfResult{
			config: {
				"TODO": 5.8,
			},
		}
	}

	err = td.Do(ctx, td.Props("Writing perf JSON file to "+outputPath), func(ctx context.Context) error {
		b, err := json.MarshalIndent(perf, "", "    ")
		if err != nil {
			return err
		}
		return ioutil.WriteFile(filepath.Join(outputPath, "perf.json"), b, 0666)
	})
	if err != nil {
		return td.FailStep(ctx, err)
	}

	return nil
}
