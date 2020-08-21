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

	// Setup.
	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	err := setup(ctx, goBuild) //flag (go build)
}

func setup(ctx context.Context, skiaCheckoutPath string) error {
	ctx = WithEnv(ctx, []string{ "MANUAL_SRC_PATH=" + skia_checkout,
								 "OSS_FUZZ_PROJECT_NAME='skia' ",
								 "REPO_NAME='skia' ",
								 filepath.Join(skiaCheckoutPath, "cifuzz.bash")})

	exec.RunCwd(ctx, "RUN wget -O skia.diff https://github.com/google/oss-fuzz/tree/master/projects/skia/skia.diff")
	exec.RunCwd(ctx, "RUN wget -O BUILD.gn.diff https://github.com/google/oss-fuzz/tree/master/projects/skia/BUILD.gn.diff")

	exec.RunCwd(ctx, "RUN git apply skia.diff")
	exec.RunCwd(ctx, "RUN cat BUILD.gn.diff >> BUILD.gn")

	ctx = td.StartStep(ctx, td.Props("setup").Infra())
	defer td.EndStep(ctx)

	if _, err := exec.RunCwd(ctx, skia_checkout, "./cifuzz.bash"); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	return nil
}
