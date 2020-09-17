// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

func main() {
	var (
		// Required properties for this task.
		gitExePath = flag.String("git_exe_path", "", "Path to a git exe. Used to checkout cifuzz repo.")
		projectID  = flag.String("project_id", "", "ID of the Google Cloud project.")
		skiaPath   = flag.String("skia_path", "", "Path to skia repo root.")
		taskID     = flag.String("task_id", "", "task id this data was generated on")
		taskName   = flag.String("task_name", "", "Name of the task.")

		// Debugging flags.
		local       = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		outputSteps = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
	)

	// Setup.
	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	// Absolute paths work more consistently than relative paths.
	skiaAbsPath := getAbsoluteOfRequiredFlag(ctx, *skiaPath, "skia_path")
	gitAbsPath := getAbsoluteOfRequiredFlag(ctx, *gitExePath, "git_exe_path")

	workdir, err := os_steps.TempDir(ctx, "", "cifuzz")
	if err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Setup cifuzz repo and images
	if err := setupCIFuzzRepoAndDocker(ctx, workdir, gitAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Checkout swiftshader to pinned version that we know works.
	if err := prepareSkiaCheckout(ctx, skiaAbsPath, gitAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// build and run fuzzers
	if err := buildAndRunCIFuzz(ctx, workdir, skiaAbsPath); err != nil {
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

const (
	ossFuzzRepo             = "https://github.com/google/oss-fuzz.git"
	dockerExe               = "docker"
	cifuzzDockerImage       = "gcr.io/oss-fuzz-base/cifuzz-base:latest"
	buildFuzzersDockerImage = "local_build_fuzzers"
	runFuzzersDockerImage   = "local_run_fuzzers"

	pinnedSwiftshaderRevision = "45510ad8a77862c1ce2e33f0efed41544f5f048b"
)

func setupCIFuzzRepoAndDocker(ctx context.Context, workdir, gitAbsPath string) error {
	ctx = td.StartStep(ctx, td.Props("setup cifuzz").Infra())
	defer td.EndStep(ctx)

	if _, err := exec.RunCwd(ctx, workdir, gitAbsPath, "clone", ossFuzzRepo, "--depth", "1"); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	if _, err := exec.RunCwd(ctx, workdir, dockerExe, "pull", cifuzzDockerImage); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	if _, err := exec.RunCwd(ctx, workdir, dockerExe, "build", "--tag", buildFuzzersDockerImage, "oss-fuzz/infra/cifuzz/actions/build_fuzzers"); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	if _, err := exec.RunCwd(ctx, workdir, dockerExe, "build", "--tag", runFuzzersDockerImage, "oss-fuzz/infra/cifuzz/actions/run_fuzzers"); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	return nil
}

func prepareSkiaCheckout(ctx context.Context, skiaAbsPath, gitAbsPath string) error {
	ctx = td.StartStep(ctx, td.Props("prepare skia checkout for build"))
	defer td.EndStep(ctx)

	swiftshaderDir := filepath.Join(skiaAbsPath, "third_party", "externals", "swiftshader")

	if _, err := exec.RunCwd(ctx, swiftshaderDir, gitAbsPath, "checkout", pinnedSwiftshaderRevision); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	return nil
}

func buildAndRunCIFuzz(ctx context.Context, workdir, skiaAbsPath string) error {
	ctx = td.StartStep(ctx, td.Props("build skia fuzzers and run them"))
	defer td.EndStep(ctx)

	if _, err := exec.RunCwd(ctx, workdir, dockerExe, "run",
		"--name", "build_fuzzers", "--rm",
		"--env", "MANUAL_SRC_PATH="+skiaAbsPath,
		"--env", "OSS_FUZZ_PROJECT_NAME=skia",
		"--env", "GITHUB_WORKSPACE="+workdir,
		"--env", "GITHUB_REPOSITORY=skia", // TODO(metzman) make this not required
		"--env", "GITHUB_EVENT_NAME=push", // TODO(metzman) make this not required
		"--env", "DRY_RUN=0",
		"--env", "CI=true",
		"--env", "SANITIZER=address",
		"--env", "GITHUB_SHA=TODO(kjlubick)",
		"--volume", "/var/run/docker.sock:/var/run/docker.sock",
		"--mount", fmt.Sprintf("type=bind,source=%s,destination=%s", skiaAbsPath, skiaAbsPath),
		"--mount", fmt.Sprintf("type=bind,source=%s,destination=%s", workdir, workdir),
		buildFuzzersDockerImage,
	); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	return nil
}
