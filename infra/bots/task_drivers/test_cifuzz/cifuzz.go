// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"path/filepath"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
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

		goBuild = flag.String("go_build", "/usr/local/google/home/zepenghu/Desktop/skia_checkout/", "")
	)
	exec.RunCwd("RUN git apply skia.diff")
	exec.RunCwd("RUN cat BUILD.gn.diff >> BUILD.gn")
	// Setup.
	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	err := setup(ctx, goBuild) //flag (go build)
}
const perfKeyCpuOrGPU = "cpu_or_gpu"

func setup(ctx context.Context, skiaCheckoutPath string) error {
	ctx = WithEnv(ctx, []string{ "MANUAL_SRC_PATH=$PWD OSS_FUZZ_PROJECT_NAME='cifuzz-example' " +
								  "REPO_NAME='cifuzz-example' COMMIT_SHA=$TRAVIS_PULL_REQUEST_SHA " +
								  filepath.Join(skiaCheckoutPath, "cifuzz.bash")})

	ctx = td.StartStep(ctx, td.Props("setup").Infra())
	defer td.EndStep(ctx)

	if _, err := exec.RunCwd(ctx, benchmarkPath, filepath.Join(skiaCheckoutPath, "cifuzz.bash")); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	return nil
}
