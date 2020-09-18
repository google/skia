// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"flag"
	"fmt"
	"path/filepath"
	"time"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

func main() {
	var (
		// Required properties for this task.
		gitExePath   = flag.String("git_exe_path", "", "Path to a git exe. Used to checkout cifuzz repo.")
		projectID    = flag.String("project_id", "", "ID of the Google Cloud project.")
		skiaPath     = flag.String("skia_path", "", "Path to skia repo root.")
		taskID       = flag.String("task_id", "", "task id this data was generated on")
		taskName     = flag.String("task_name", "", "Name of the task.")
		workPath     = flag.String("work_path", "", "The directory to use to store temporary files (e.g. fuzzers)")
		outPath      = flag.String("out_path", "", "The directory to put any crashes/hangs/outputs found.")
		fuzzDuration = flag.Duration("fuzz_duration", 600*time.Second, "The total time that the fuzzers run. Divided up between all fuzzers.")

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
	workAbsPath := getAbsoluteOfRequiredFlag(ctx, *workPath, "work_path")
	outAbsPath := getAbsoluteOfRequiredFlag(ctx, *outPath, "out_path")

	workDir := filepath.Join(workAbsPath, "cifuzz")
	if err := os_steps.MkdirAll(ctx, workDir); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Setup cifuzz repo and images
	if err := setupCIFuzzRepoAndDocker(ctx, workDir, gitAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Prepare the skia checkout to be built with fuzzers.
	if err := prepareSkiaCheckout(ctx, skiaAbsPath, workDir, gitAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// build and run fuzzers
	if err := buildAndRunCIFuzz(ctx, workDir, *fuzzDuration); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	if err := extractOutput(ctx, workDir, outAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// Clean up compiled fuzzers, etc
	if err := os_steps.RemoveAll(ctx, workDir); err != nil {
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

	// Make these directories for cifuzz exist so docker does not create it w/ root permissions.
	if err := os_steps.MkdirAll(ctx, filepath.Join(workdir, "out")); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

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

func prepareSkiaCheckout(ctx context.Context, skiaAbsPath, workDir, gitAbsPath string) error {
	ctx = td.StartStep(ctx, td.Props("prepare skia checkout for build"))
	defer td.EndStep(ctx)

	// Docker needs to copy the source into the docker container in order to do the compile. Thus,
	// we don't want it to copy things like binaries in the out directory. As such, we make a copy
	// of the skia checkout in the workdir: https://superuser.com/a/1520540
	skiaCopyDir := filepath.Join(workDir, "copy", "skia")

	if _, err := exec.RunCwd(ctx, workDir, gitAbsPath, "clone", skiaAbsPath, skiaCopyDir); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	// We also need to bin sync
	if _, err := exec.RunCwd(ctx, skiaCopyDir, "python", filepath.Join("bin", "sync")); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	swiftshaderDir := filepath.Join(skiaCopyDir, "third_party", "externals", "swiftshader")

	if _, err := exec.RunCwd(ctx, swiftshaderDir, gitAbsPath, "checkout", pinnedSwiftshaderRevision); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	return nil
}

func buildAndRunCIFuzz(ctx context.Context, workDir string, duration time.Duration) error {
	ctx = td.StartStep(ctx, td.Props("build skia fuzzers and run them"))
	defer td.EndStep(ctx)

	skiaCopyDir := filepath.Join(workDir, "copy", "skia")

	// See https://google.github.io/oss-fuzz/getting-started/continuous-integration/#optional-configuration
	if _, err := exec.RunCwd(ctx, workDir, dockerExe, "run",
		"--name", "build_fuzzers", "--rm",
		"--env", "MANUAL_SRC_PATH="+skiaCopyDir,
		"--env", "OSS_FUZZ_PROJECT_NAME=skia",
		"--env", "GITHUB_WORKSPACE="+workDir,
		"--env", "GITHUB_REPOSITORY=skia", // TODO(metzman) make this not required
		"--env", "GITHUB_EVENT_NAME=push", // TODO(metzman) make this not required
		"--env", "DRY_RUN=false",
		"--env", "CI=true",
		"--env", "SANITIZER=address",
		"--env", "GITHUB_SHA=does_nothing",
		"--volume", "/var/run/docker.sock:/var/run/docker.sock",
		"--mount", fmt.Sprintf("type=bind,source=%s,destination=%s", skiaCopyDir, skiaCopyDir),
		"--mount", fmt.Sprintf("type=bind,source=%s,destination=%s", workDir, workDir),
		buildFuzzersDockerImage,
	); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	if _, err := exec.RunCwd(ctx, workDir, dockerExe, "run",
		"--name", "run_fuzzers", "--rm",
		"--env", "OSS_FUZZ_PROJECT_NAME=skia",
		"--env", "GITHUB_WORKSPACE="+workDir,
		"--env", "GITHUB_REPOSITORY=skia", // TODO(metzman) make this not required
		"--env", "GITHUB_EVENT_NAME=push", // TODO(metzman) make this not required
		"--env", "DRY_RUN=false",
		"--env", "CI=true",
		"--env", "FUZZ_TIME="+fmt.Sprintf("%d", duration/time.Second), // This is split up between all affected fuzzers.
		"--env", "SANITIZER=address",
		"--env", "GITHUB_SHA=does_nothing",
		"--volume", "/var/run/docker.sock:/var/run/docker.sock",
		"--mount", fmt.Sprintf("type=bind,source=%s,destination=%s", workDir, workDir),
		runFuzzersDockerImage,
	); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	cifuzzOutDir := filepath.Join(workDir, "out")

	// Fix up permissions of output directory (we need to delete extra folders here so we can
	// clean up after we copy out the crash/hang files).
	if _, err := exec.RunCwd(ctx, workDir, dockerExe, "run",
		"--mount", fmt.Sprintf("type=bind,source=%s,destination=/OUT", cifuzzOutDir),
		cifuzzDockerImage,
		"/bin/bash", "-c", `rm -rf /OUT/*/ && chmod 0666 /OUT/*`,
	); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	return nil
}

func extractOutput(ctx context.Context, workDir, outAbsPath string) error {
	ctx = td.StartStep(ctx, td.Props("copy output directory"))
	defer td.EndStep(ctx)

	// Make these directories for cifuzz exist so docker does not create it w/ root permissions.
	if err := os_steps.MkdirAll(ctx, outAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	cifuzzOutDir := filepath.Join(workDir, "out")

	if _, err := exec.RunCwd(ctx, workDir, dockerExe, "run",
		"--mount", fmt.Sprintf("type=bind,source=%s,destination=/WORK", cifuzzOutDir),
		"--mount", fmt.Sprintf("type=bind,source=%s,destination=/OUT", outAbsPath),
		cifuzzDockerImage,
		"/bin/bash", "-c", `cp /WORK/* /OUT/`,
	); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}
	return nil
}
