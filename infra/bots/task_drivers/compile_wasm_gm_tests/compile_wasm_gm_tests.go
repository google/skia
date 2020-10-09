// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"context"
	"flag"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"

	"cloud.google.com/go/storage"
	"go.skia.org/infra/go/auth"
	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/skerr"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/task_driver/go/lib/auth_steps"
	"go.skia.org/infra/task_driver/go/lib/docker"
	"go.skia.org/infra/task_driver/go/lib/os_steps"
	"go.skia.org/infra/task_driver/go/td"
)

const dockerImage = "gcr.io/skia-public/canvaskit-emsdk:2.0.6_v1"
const innerBuildScript = "/SRC/infra/canvaskit/build_gmtests.sh"

func main() {
	var (
		// Required properties for this task.
		outPath   = flag.String("out_path", "", "The directory to put the built wasm/js code.")
		projectID = flag.String("project_id", "", "ID of the Google Cloud project.")
		skiaPath  = flag.String("skia_path", "", "Path to skia repo root.")
		taskID    = flag.String("task_id", "", "task id this data was generated on")
		taskName  = flag.String("task_name", "", "Name of the task.")
		workPath  = flag.String("work_path", "", "The directory to use to store temporary files (e.g. docker build)")

		// Debugging flags.
		local       = flag.Bool("local", false, "True if running locally (as opposed to on the bots)")
		outputSteps = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")
	)

	// Setup.
	ctx := td.StartRun(projectID, taskID, taskName, outputSteps, local)
	defer td.EndRun(ctx)

	outAbsPath := getAbsoluteOfRequiredFlag(ctx, *outPath, "out_path")
	skiaAbsPath := getAbsoluteOfRequiredFlag(ctx, *skiaPath, "skia_path")
	workAbsPath := getAbsoluteOfRequiredFlag(ctx, *workPath, "work_path")

	if err := os_steps.MkdirAll(ctx, workAbsPath); err != nil {
		td.Fatal(ctx, err)
	}
	if err := os_steps.MkdirAll(ctx, outAbsPath); err != nil {
		td.Fatal(ctx, err)
	}

	configDir, err := setupDocker(ctx, *local)
	defer util.RemoveAll(configDir)
	if err != nil {
		td.Fatal(ctx, err)
	}

	// FIXME remove debugging
	if _, err := exec.RunCwd(ctx, skiaAbsPath, "ls", "-ahl"); err != nil {
		td.Fatal(ctx, err)
	}

	command := []string{innerBuildScript}
	volumes := []string{skiaAbsPath + ":/SRC", workAbsPath + ":/OUT"}

	if err := docker.Run(ctx, dockerImage, configDir, command, volumes, nil); err != nil {
		td.Fatal(ctx, err)
	}

	if err := extractOutput(ctx, workAbsPath, outAbsPath); err != nil {
		td.Fatal(ctx, err)
	}
}

func setupDocker(ctx context.Context, isLocal bool) (string, error) {
	ctx = td.StartStep(ctx, td.Props("setup docker").Infra())
	defer td.EndStep(ctx)
	// Create token source with scope for cloud registry (storage).
	ts, err := auth_steps.Init(ctx, isLocal, auth.SCOPE_USERINFO_EMAIL, storage.ScopeReadOnly)
	if err != nil {
		return "", td.FailStep(ctx, err)
	}

	// Create a temporary config dir for Docker.
	configDir, err := ioutil.TempDir("", "")
	if err != nil {
		return "", td.FailStep(ctx, err)
	}

	// Authenticate docker with GKE (required to pull from container registry).
	token, err := ts.Token()
	if err != nil {
		return "", td.FailStep(ctx, err)
	}
	if err := docker.Login(ctx, token.AccessToken, "gcr.io/skia-public/", configDir); err != nil {
		return "", td.FailStep(ctx, err)
	}
	return configDir, nil
}

func getAbsoluteOfRequiredFlag(ctx context.Context, nonEmptyPath, flag string) string {
	if nonEmptyPath == "" {
		td.Fatalf(ctx, "--%s must be specified", flag)
	}
	absPath, err := filepath.Abs(nonEmptyPath)
	if err != nil {
		td.Fatalf(ctx, "error with path %s - %s", nonEmptyPath, err)
	}
	return absPath
}

func extractOutput(ctx context.Context, workDir, outAbsPath string) error {
	ctx = td.StartStep(ctx, td.Props("copy compiled JS and wasm into output"))
	defer td.EndStep(ctx)

	files, err := os_steps.ReadDir(ctx, workDir)
	if err != nil {
		return td.FailStep(ctx, skerr.Wrapf(err, "getting output from %s", workDir))
	}

	for _, f := range files {
		name := f.Name()
		if strings.Contains(name, "wasm_gm_tests") {
			oldFile := filepath.Join(workDir, name)
			newFile := filepath.Join(outAbsPath, name)
			if err := os.Rename(oldFile, newFile); err != nil {
				return td.FailStep(ctx, skerr.Wrapf(err, "copying %s to %s", oldFile, newFile))
			}
		}
	}
	return nil
}
