// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/task_driver/go/td"
)

func main() {
	var (
		// Required properties for this task.
		projectId     = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId        = flag.String("task_id", "", "ID of this task.")
		taskName      = flag.String("task_name", "", "Name of the task.")
		benchmarkPath = flag.String("benchmark_path", "", "Path to location of the benchmark files.")
		lottiesPath   = flag.String("lotties_path", "", "Path to location of lottie files.")
		workdir       = flag.String("workdir", ".", "Working directory")
		outputPath    = flag.String("output_path", "", "Perf Output will be produced here")

		// Optional flags.
		local        = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		outputSteps  = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
		canvaskitBin = flag.String("canvaskit_bin", "", "The location of a canvaskit.js and canvaskit.wasm")
	)

	// Setup.
	ctx := td.StartRun(projectId, taskId, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	// FIXME(kjlubick) Remove this when done.
	if _, err := exec.RunCwd(ctx, *workdir, "ls", "-ahl"); err != nil {
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
		if err := processSkottieFramesData(ctx, *benchmarkPath, *outputPath); err != nil {
			td.Fatal(ctx, err)
		}
	default:
		td.Fatalf(ctx, "Unknown task config %s [%s]", config, *taskName)
	}

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

func benchSkottieFrames(ctx context.Context, benchmarkPath string, canvasKitBin string, lottiesPath string) error {
	ctx = td.StartStep(ctx, td.Props("perf lotties in "+lottiesPath))
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
		break // FIXME(kjlubick)
	}

	return nil
}

func processSkottieFramesData(ctx context.Context, s string, s2 string) error {
	return nil
}
