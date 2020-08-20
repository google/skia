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

func main() {
	var (
		// Required properties for this task.
		projectID     = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskName      = flag.String("task_name", "", "Name of the task.")
		gitHash       = flag.String("git_hash", "", "Git hash this data corresponds to")
		taskID        = flag.String("task_id", "", "task id this data was generated on")
		skia_checkout = flag.String("go_build", "", "Skia checkout")
	)
	exec.RunCwd("RUN wget -O skia.diff https://github.com/google/oss-fuzz/tree/master/projects/skia/skia.diff")
	exec.RunCwd("RUN wget -O BUILD.gn.diff https://github.com/google/oss-fuzz/tree/master/projects/skia/BUILD.gn.diff")

	exec.RunCwd("RUN git apply skia.diff")
	exec.RunCwd("RUN cat BUILD.gn.diff >> BUILD.gn")
	// Setup.
	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	err := setup(ctx, goBuild) //flag (go build)
}
const perfKeyCpuOrGPU = "cpu_or_gpu"

func setup(ctx context.Context, skiaCheckoutPath string) error {
	ctx = WithEnv(ctx, []string{ "MANUAL_SRC_PATH=" + skia_checkout +
								 "OSS_FUZZ_PROJECT_NAME='skia' " +
								 "REPO_NAME='skia' " +
								 filepath.Join(skiaCheckoutPath, "cifuzz.bash")})

	ctx = td.StartStep(ctx, td.Props("setup").Infra())
	defer td.EndStep(ctx)

	if _, err := exec.RunCwd(ctx, benchmarkPath, "./cifuzz.bash"); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	return nil
}
