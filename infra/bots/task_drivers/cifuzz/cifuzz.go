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
	"time"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/git/git_common"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/sklog"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

var sleepOnFail = flag.Bool("sleep_on_fail", false, "True if we should sleep for 30 minutes on failure instead of exiting (for inspection via SSH)")

func main() {
	var (
		// Required properties for this task.
		fuzzDuration = flag.Duration("fuzz_duration", 600*time.Second, "The total time that the fuzzers run. Divided up between all fuzzers.")
		gitExePath   = flag.String("git_exe_path", "", "Path to a git exe. Used to checkout cifuzz repo.")
		outPath      = flag.String("out_path", "", "The directory to put any crashes/hangs/outputs found.")
		projectID    = flag.String("project_id", "", "ID of the Google Cloud project.")
		skiaPath     = flag.String("skia_path", "", "Path to skia repo root.")
		taskID       = flag.String("task_id", "", "task id this data was generated on")
		taskName     = flag.String("task_name", "", "Name of the task.")
		workPath     = flag.String("work_path", "", "The directory to use to store temporary files (e.g. fuzzers)")

		// Debugging flags.
		local       = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		outputSteps = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
	)

	// Setup.
	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	// Absolute paths work more consistently than relative paths.
	gitAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *gitExePath, "git_exe_path")
	outAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *outPath, "out_path")
	skiaAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *skiaPath, "skia_path")
	workAbsPath := td.MustGetAbsolutePathOfFlag(ctx, *workPath, "work_path")

	if !git_common.IsFromCIPD(gitAbsPath) {
		fatalOrSleep(ctx, skerr.Fmt("Git %s must be from CIPD", gitAbsPath))
	}

	workDir := filepath.Join(workAbsPath, "cifuzz")
	if err := os_steps.MkdirAll(ctx, workDir); err != nil {
		fatalOrSleep(ctx, skerr.Wrap(err))
	}

	// Setup cifuzz repo and images
	if err := setupCIFuzzRepoAndDocker(ctx, workDir, gitAbsPath); err != nil {
		fatalOrSleep(ctx, skerr.Wrap(err))
	}

	// Prepare the skia checkout to be built with fuzzers.
	if err := prepareSkiaCheckout(ctx, skiaAbsPath, workDir, gitAbsPath); err != nil {
		td.Fatal(ctx, skerr.Wrap(err))
	}

	// build and run fuzzers. If it fails (errors), hold that until we cleanup and copy the output.
	// That way, developers can have access to the crashes.
	runErr := buildAndRunCIFuzz(ctx, workDir, skiaAbsPath, *fuzzDuration)

	if err := extractOutput(ctx, workDir, outAbsPath); err != nil {
		fatalOrSleep(ctx, skerr.Wrap(err))
	}

	// Clean up compiled fuzzers, etc
	if err := os_steps.RemoveAll(ctx, workDir); err != nil {
		fatalOrSleep(ctx, skerr.Wrap(err))
	}

	if runErr != nil {
		fatalOrSleep(ctx, skerr.Wrap(runErr))
	}
}

func fatalOrSleep(ctx context.Context, err error) {
	if *sleepOnFail {
		sklog.Errorf("Sleeping after error: %s", err)
		time.Sleep(30 * time.Minute)
	}
	td.Fatal(ctx, err)
}

const (
	ossFuzzRepo             = "https://github.com/google/oss-fuzz.git"
	swiftShaderRepo         = "https://swiftshader.googlesource.com/SwiftShader"
	dockerExe               = "docker"
	cifuzzDockerImage       = "gcr.io/oss-fuzz-base/cifuzz-base:latest"
	buildFuzzersDockerImage = "local_build_fuzzers"
	runFuzzersDockerImage   = "local_run_fuzzers"
)

func setupCIFuzzRepoAndDocker(ctx context.Context, workdir, gitAbsPath string) error {
	ctx = td.StartStep(ctx, td.Props("setup cifuzz").Infra())
	defer td.EndStep(ctx)

	// Make these directories for cifuzz exist so docker does not create it w/ root permissions.
	if err := os_steps.MkdirAll(ctx, filepath.Join(workdir, "out")); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	if _, err := exec.RunCwd(ctx, workdir, gitAbsPath, "clone", ossFuzzRepo, "--depth", "1"); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	if _, err := exec.RunCwd(ctx, workdir, dockerExe, "pull", cifuzzDockerImage); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	ossFuzzDir := filepath.Join(workdir, "oss-fuzz", "infra")

	if _, err := exec.RunCwd(ctx, ossFuzzDir, dockerExe, "build", "--tag", buildFuzzersDockerImage,
		"--file", "build_fuzzers.Dockerfile", "."); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	if _, err := exec.RunCwd(ctx, ossFuzzDir, dockerExe, "build", "--tag", runFuzzersDockerImage,
		"--file", "run_fuzzers.Dockerfile", "."); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	return nil
}

func prepareSkiaCheckout(ctx context.Context, skiaAbsPath, workDir, gitAbsPath string) error {
	ctx = td.StartStep(ctx, td.Props("prepare skia checkout for build").Infra())
	defer td.EndStep(ctx)

	swiftshaderDir := filepath.Join(skiaAbsPath, "third_party", "externals", "swiftshader")

	if _, err := exec.RunCwd(ctx, workDir, "rm", "-rf", swiftshaderDir); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	// We have to clone swiftshader *and* its deps (which are not DEPS, but git submodules) in order
	// to build it with fuzzers.
	if _, err := exec.RunCwd(ctx, skiaAbsPath, gitAbsPath, "clone", "--recursive", swiftShaderRepo, swiftshaderDir); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	return nil
}

func buildAndRunCIFuzz(ctx context.Context, workDir, skiaAbsPath string, duration time.Duration) error {
	ctx = td.StartStep(ctx, td.Props("build skia fuzzers and run them"))
	defer td.EndStep(ctx)

	// See https://google.github.io/oss-fuzz/getting-started/continuous-integration/#optional-configuration
	if _, err := exec.RunCwd(ctx, workDir, dockerExe, "run",
		"--name", "build_fuzzers", "--rm",
		"--env", "MANUAL_SRC_PATH="+skiaAbsPath,
		"--env", "OSS_FUZZ_PROJECT_NAME=skia",
		"--env", "GITHUB_WORKSPACE="+workDir,
		"--env", "GITHUB_REPOSITORY=skia", // TODO(metzman) make this not required
		"--env", "GITHUB_EVENT_NAME=push", // TODO(metzman) make this not required
		"--env", "DRY_RUN=false",
		"--env", "CI=true",
		"--env", "CIFUZZ=true",
		"--env", "SANITIZER=address",
		"--env", "GITHUB_SHA=does_nothing",
		"--volume", "/var/run/docker.sock:/var/run/docker.sock",
		"--mount", fmt.Sprintf("type=bind,source=%s,destination=%s", skiaAbsPath, skiaAbsPath),
		"--mount", fmt.Sprintf("type=bind,source=%s,destination=%s", workDir, workDir),
		buildFuzzersDockerImage,
	); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	args := []string{"run",
		"--name", "run_fuzzers", "--rm",
		"--env", "OSS_FUZZ_PROJECT_NAME=skia",
		"--env", "GITHUB_WORKSPACE=" + workDir,
		"--env", "GITHUB_REPOSITORY=skia", // TODO(metzman) make this not required
		"--env", "GITHUB_EVENT_NAME=push", // TODO(metzman) make this not required
		"--env", "DRY_RUN=false",
		"--env", "CI=true",
		"--env", "CIFUZZ=true",
		"--env", "FUZZ_TIME=" + fmt.Sprintf("%d", duration/time.Second), // This is split up between all affected fuzzers.
		"--env", "SANITIZER=address",
		"--env", "GITHUB_SHA=does_nothing",
		"--volume", "/var/run/docker.sock:/var/run/docker.sock",
		"--mount", fmt.Sprintf("type=bind,source=%s,destination=%s", workDir, workDir),
		runFuzzersDockerImage,
	}

	cmd := exec.Command{
		Name:    dockerExe,
		Args:    args,
		Dir:     workDir,
		Timeout: duration + 10*time.Minute, // Give a little padding in case fuzzing takes some extra time.
	}
	if _, err := exec.RunCommand(ctx, &cmd); err != nil {
		if !exec.IsTimeout(err) {
			return td.FailStep(ctx, skerr.Wrap(err))
		} else {
			sklog.Warningf("Fuzzing timed out: %s", err)
		}
	}
	return nil
}

func extractOutput(ctx context.Context, workDir, outAbsPath string) error {
	ctx = td.StartStep(ctx, td.Props("copy output directory").Infra())
	defer td.EndStep(ctx)

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

	// Make these directories for cifuzz exist so docker does not create it w/ root permissions.
	if err := os_steps.MkdirAll(ctx, outAbsPath); err != nil {
		return td.FailStep(ctx, skerr.Wrap(err))
	}

	files, err := os_steps.ReadDir(ctx, cifuzzOutDir)
	if err != nil {
		return td.FailStep(ctx, skerr.Wrapf(err, "getting output from %s", cifuzzOutDir))
	}

	for _, f := range files {
		name := f.Name()
		if strings.Contains(name, "crash-") || strings.Contains(name, "oom-") || strings.Contains(name, "timeout-") {
			oldFile := filepath.Join(cifuzzOutDir, name)
			newFile := filepath.Join(outAbsPath, name)
			if err := os.Rename(oldFile, newFile); err != nil {
				return td.FailStep(ctx, skerr.Wrapf(err, "copying %s to %s", oldFile, newFile))
			}
		}
	}

	return nil
}
